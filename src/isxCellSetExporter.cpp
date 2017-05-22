#include "isxCellSetExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"

#include <fstream>
#include <iomanip>
#include <limits>


namespace isx {

std::string
CellSetExporterParams::getOpName()
{
    return "Export CellSet";
}
    
AsyncTaskStatus 
runCellSetExporter(CellSetExporterParams inParams, std::shared_ptr<CellSetExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    const int32_t timeDecimals = std::numeric_limits<double>::digits10 + 1;
    const int32_t maxDecimalsForFloat = std::numeric_limits<float>::digits10 + 1;
    
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
    if(inParams.m_outputTraceFilename.empty() == false)
    {
        std::ofstream strm(inParams.m_outputTraceFilename, std::ios::trunc);

        if (!strm.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
        }
        
        isx::DurationInSeconds baseTime{};
        switch (inParams.m_writeTimeRelativeTo)
        {
            case WriteTimeRelativeTo::FIRST_DATA_ITEM:
            {
                baseTime = srcs[0]->getTimingInfo().getStart().getSecsSinceEpoch();;
                break;
            }
            case WriteTimeRelativeTo::UNIX_EPOCH:
            {
                baseTime = isx::DurationInSeconds{};
                break;
            }
            default:
                ISX_THROW(isx::ExceptionUserInput, "Invalid setting for writeTimeRelativeTo");
        }

        // calculate total number of lines to write (for progress reporting)
        isize_t numLinesTotal = 0; 
        isize_t numLinesWritten = 0;
        for (auto & cs: srcs)
        {
            numLinesTotal += cs->getTimingInfo().getNumTimes();
        }

        // write column headers
        strm << " , ";
        for (isize_t i = 0; i < numCells - 1; ++i)
        {
            strm << srcs[0]->getCellName(i) << ", ";
        }
        strm << srcs[0]->getCellName(numCells - 1) << "\n";

        // write cell statuses
        strm << "Time(s)/Cell Status, ";
        for (isize_t i = 0; i < numCells - 1; ++i)
        {
            strm << srcs[0]->getCellStatusString(i) << ", ";
        }
        strm << srcs[0]->getCellStatusString(numCells - 1) << "\n";
        
        // iterate over input cell sets
        for (auto & cs: srcs)
        {
            // loop over samples
            const isize_t numSamples = cs->getTimingInfo().getNumTimes();
            for (isize_t sample = 0; sample < numSamples; ++sample)
            {
                // write time point
                {
                    auto tm = cs->getTimingInfo().convertIndexToStartTime(sample).getSecsSinceEpoch();
                    auto timeToWrite = (tm - baseTime).toDouble();
                    strm << std::setprecision(timeDecimals);
                    strm << timeToWrite << ", ";
                }
                
                strm << std::setprecision(maxDecimalsForFloat);

                // loop over individual cells
                for (isize_t cell = 0; cell < numCells - 1; ++cell)
                {
                    // write cell's data value
                    auto tr = cs->getTrace(cell);
                    strm << tr->getValue(sample) << ", ";
                }
                // write last cell's data value
                auto tr = cs->getTrace(numCells - 1);
                strm << tr->getValue(sample);

                // write newline before next time point
                strm << "\n";
                ++numLinesWritten;
                progress = float(numLinesWritten) / float(numLinesTotal) / float(numSections);
                cancelled = inCheckInCB(progress);
                if (cancelled)
                {
                    break;
                }
            }
            if (cancelled)
            {
                break;
            }
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
            toTiff(fn, cellIm);    

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
