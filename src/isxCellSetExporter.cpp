#include "isxCellSetExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxExportPNG.h"
#include "isxCellSetUtils.h"

#include <fstream>
#include <iomanip>
#include <limits>

#include "json.hpp"

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

    // For progress report
    isize_t numSections = isize_t(inParams.m_outputTraceFilename.empty() == false) +
                            isize_t(inParams.m_outputImageFilename.empty() == false);
    float progress = 0.f;

    /// Traces to CSV
    if (inParams.m_outputTraceFilename.empty() == false)
    {
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
            cancelled = writeTraces(strm, traces, names, statuses, baseTime, inCheckInCB);
        }
        catch (...)
        {
            strm.close();
            std::remove(inParams.m_outputTraceFilename.c_str());
            throw;
        }
    }

    /// Images to TIFF
    if(inParams.m_outputImageFilename.empty() == false)
    {
        // If many srcs are provided, it's enough to use the first one only since
        // cell images in a cellset series are the same for different segments
        auto & cs =  srcs[0];
        std::string dirname = getDirName(inParams.m_outputImageFilename);
        std::string basename = getBaseName(inParams.m_outputImageFilename);
        std::string extension = getExtension(inParams.m_outputImageFilename);

        for (isize_t cell = 0; cell < numCells; ++cell)
        {
            std::string cellname = cs->getCellName(cell);
            std::string fn = dirname + "/" + basename + "_" + cellname + "." + extension;

            SpImage_t cellIm = cs->getImage(cell);

            try
            {
                toTiff(fn, cellIm);
            }
            catch (...)
            {
                for (isize_t c = 0; c <= cell; ++c)
                {
                    std::string cellname = cs->getCellName(c);
                    std::string fn = dirname + "/" + basename + "_" + cellname + "." + extension;
                    std::remove(fn.c_str());
                }
                throw;
            }


            cancelled = inCheckInCB(progress + float(cell)/float(numCells)/float(numSections));
            if (cancelled)
            {
                // Remove previously created files - Do this here and not in finishedCB because
                // only here we know exactly which files we wrote out
                for (isize_t c = 0; c <= cell; ++c)
                {
                    std::string cellname = cs->getCellName(c);
                    std::string fn = dirname + "/" + basename + "_" + cellname + "." + extension;
                    std::remove(fn.c_str());
                }
                break;
            }
        }

        if (!cancelled)
        {
            // export accepted cell map to tiff
            SpImage_t map = cellSetToCellMap(cs, false, true);
            std::string fn = dirname + "/" + basename + "_accepted-cells-map." + extension;
            toTiff(fn, map);

            // export accepted cell map to png
            if (inParams.m_writePngImage)
            {
                SpImage_t PNGmap = convertImageF32toU8(map);
                fn = dirname + "/" + basename + "_accepted-cells-map.png";
                toPng(fn, PNGmap);
            }
        }
    }

    if (cancelled)
    {
        if(!inParams.m_outputTraceFilename.empty())
        {
            std::remove(inParams.m_outputTraceFilename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
