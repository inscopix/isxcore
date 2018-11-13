#include "isxCellSetExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxExportPNG.h"
#include "isxCellSetUtils.h"
#include "isxCellSetFactory.h"

#include <fstream>
#include <iomanip>
#include <limits>

#include "json.hpp"

namespace
{

bool
writeCellProperties(
        const std::vector<isx::SpCellSet_t> & inCellSets,
        const std::string & inFilePath,
        isx::AsyncCheckInCB_t inCheckInCB)
{
    // Read the cellsets as a series to be consistent with the GUI's report of metrics.
    std::vector<std::string> csFiles;
    for (const auto & cs : inCellSets)
    {
        csFiles.push_back(cs->getFileName());
    }
    const isx::SpCellSet_t csSeries = isx::readCellSetSeries(csFiles);
    const size_t numSegments = csSeries->getTimingInfosForSeries().size();
    const bool hasMetrics = csSeries->hasMetrics();

    std::ofstream csv(inFilePath);
    csv << "Name,Status,ColorR,ColorG,ColorB";
    if (hasMetrics)
    {
        csv << ",CentroidX,CentroidY,NumComponents,Size";
    }
    for (size_t i = 0; i < numSegments; ++i)
    {
        csv << ",ActiveSegment" << i;
    }
    csv << std::endl;

    const isx::isize_t numCells = csSeries->getNumCells();
    for (isx::isize_t c = 0; c < numCells; ++c)
    {
        const isx::Color color = csSeries->getCellColor(c);

        csv << csSeries->getCellName(c)
            << "," << csSeries->getCellStatusString(c)
            << "," << int(color.getRed())
            << "," << int(color.getGreen())
            << "," << int(color.getBlue());

        if (hasMetrics)
        {
            const isx::SpImageMetrics_t imageMetrics = csSeries->getImageMetrics(c);
            const isx::PointInPixels_t & center = imageMetrics->m_largestComponentCenterInPixels;
            csv << "," << center.getX()
                << "," << center.getY()
                << "," << imageMetrics->m_numComponents
                << "," << imageMetrics->m_largestComponentMaxContourWidthInPixels;
        }

        ISX_ASSERT(csSeries->getCellActivity(c).size() == numSegments);
        for (const auto a : csSeries->getCellActivity(c))
        {
            csv << "," << int(a);
        }

        if (inCheckInCB(float(c) / float(numCells)))
        {
            return true;
        }

        csv << std::endl;
    }
    return false;
}

} // namespace

namespace isx {

std::string
CellSetExporterParams::getOpName()
{
    return "Export Cell Set";
}

std::string
CellSetExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["writeTimeRelativeTo"] = int(m_writeTimeRelativeTo);
    j["writePngImage"] = m_writePngImage;
    return j.dump(4);
}

std::vector<std::string>
CellSetExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
CellSetExporterParams::getOutputFilePaths() const
{
    return {m_outputTraceFilename, m_outputImageFilename};
}

AsyncTaskStatus
runCellSetExporter(CellSetExporterParams inParams, std::shared_ptr<CellSetExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;
    auto & srcs = inParams.m_srcs;

    // validate inputs
    if (srcs.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    for (auto & cs: srcs)
    {
        if (cs == nullptr)
        {
            ISX_THROW(isx::ExceptionUserInput, "One or more of the sources is invalid.");
        }
    }

    const isize_t numCells = srcs[0]->getNumCells();
    for (auto & cs: srcs)
    {
        if (cs->getNumCells() != numCells)
        {
            ISX_THROW(isx::ExceptionUserInput, "Number of cells has to be the same for all input files.");
        }
    }

    if (inParams.m_autoOutputProps && inParams.m_propertiesFilename.empty() && !inParams.m_outputTraceFilename.empty())
    {
        inParams.m_propertiesFilename = makeOutputFilePath(inParams.m_outputTraceFilename, "-props.csv");
    }

    // For progress report
    const bool outputTraces = !inParams.m_outputTraceFilename.empty();
    const bool outputImages = !inParams.m_outputImageFilename.empty();
    const bool outputProps = !inParams.m_propertiesFilename.empty();

    std::vector<std::string> filesToCleanUp;

    /// Traces to CSV
    if (outputTraces)
    {
        float checkInOffset = 0.f;
        float checkInScale = 1.f;
        if (outputImages)
        {
            checkInScale = outputProps ? 0.4f : 0.5f;
        }
        else if (outputProps)
        {
            checkInScale = 0.8f;
        }
        AsyncCheckInCB_t tracesCheckInCB = rescaleCheckInCB(inCheckInCB, checkInOffset, checkInScale);

        filesToCleanUp.push_back(inParams.m_outputTraceFilename);

        std::ofstream strm(inParams.m_outputTraceFilename, std::ios::trunc);

        if (!strm.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
        }

        const SpCellSet_t & refSrc = srcs.front();
        isx::Time baseTime;
        switch (inParams.m_writeTimeRelativeTo)
        {
            case WriteTimeRelativeTo::FIRST_DATA_ITEM:
            {
                baseTime = refSrc->getTimingInfo().getStart();
                break;
            }
            case WriteTimeRelativeTo::UNIX_EPOCH:
            {
                // baseTime is already the unix epoch
                break;
            }
            default:
                ISX_THROW(ExceptionUserInput, "Invalid setting for writeTimeRelativeTo");
        }

        std::vector<std::vector<SpFTrace_t>> traces(srcs.size());
        std::vector<std::string> names;
        std::vector<std::string> statuses;

        const isize_t numCells = refSrc->getNumCells();
        for (isize_t c = 0; c < numCells; ++c)
        {
            names.push_back(refSrc->getCellName(c));
            statuses.push_back(refSrc->getCellStatusString(c));
            for (size_t s = 0; s < srcs.size(); ++s)
            {
                traces[s].push_back(srcs[s]->getTrace(c));
            }
        }

        try
        {
            cancelled = writeTraces(strm, traces, names, statuses, baseTime, tracesCheckInCB);
        }
        catch (...)
        {
            strm.close();
            removeFiles(filesToCleanUp);
            throw;
        }
    }

    /// Images to TIFF
    if (outputImages)
    {
        float checkInOffset = 0.f;
        float checkInScale = 1.f;
        if (outputTraces)
        {
            checkInOffset = outputProps ? 0.4f : 0.5f;
            checkInScale = outputProps ? 0.4f : 0.5f;
        }
        else if (outputProps)
        {
            checkInScale = 0.8f;
        }
        AsyncCheckInCB_t imagesCheckInCB = rescaleCheckInCB(inCheckInCB, checkInOffset, checkInScale);

        // If many srcs are provided, it's enough to use the first one only since
        // cell images in a cellset series are the same for different segments
        auto & cs =  srcs[0];
        std::string dirname = getDirName(inParams.m_outputImageFilename);
        std::string basename = getBaseName(inParams.m_outputImageFilename);
        std::string extension = getExtension(inParams.m_outputImageFilename);

        for (isize_t cell = 0; cell < numCells; ++cell)
        {
            const std::string cellname = cs->getCellName(cell);
            const std::string fn = dirname + "/" + basename + "_" + cellname + "." + extension;

            filesToCleanUp.push_back(fn);

            SpImage_t cellIm = cs->getImage(cell);

            try
            {
                toTiff(fn, cellIm);
            }
            catch (...)
            {
                removeFiles(filesToCleanUp);
                throw;
            }

            cancelled = imagesCheckInCB(float(cell) / float(numCells));
            if (cancelled)
            {
                break;
            }
        }

        if (!cancelled)
        {
            // export accepted cell map to tiff
            SpImage_t map = cellSetToCellMap(cs, false, true);
            std::string fn = dirname + "/" + basename + "_accepted-cells-map." + extension;
            filesToCleanUp.push_back(fn);
            toTiff(fn, map);

            // export accepted cell map to png
            if (inParams.m_writePngImage)
            {
                SpImage_t PNGmap = convertImageF32toU8(map);
                fn = dirname + "/" + basename + "_accepted-cells-map.png";
                filesToCleanUp.push_back(fn);
                toPng(fn, PNGmap);
            }
        }
    }

    /// Cell properties to CSV
    if (outputProps)
    {
        const bool outputOther = outputTraces || outputImages;
        AsyncCheckInCB_t propsCheckInCB = rescaleCheckInCB(inCheckInCB, outputOther ? 0.8f : 0.f, outputOther ? 0.2f : 1.f);

        filesToCleanUp.push_back(inParams.m_propertiesFilename);
        try
        {
            cancelled = writeCellProperties(srcs, inParams.m_propertiesFilename, propsCheckInCB);
        }
        catch (...)
        {
            removeFiles(filesToCleanUp);
            throw;
        }
    }

    if (cancelled)
    {
        removeFiles(filesToCleanUp);
        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
