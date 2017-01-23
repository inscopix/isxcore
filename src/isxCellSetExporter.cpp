#include "isxCellSetExporter.h"


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
runCellSetExporter(CellSetExporterParams inParams, AsyncCheckInCB_t inCheckInCB)
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

    std::ofstream strm(inParams.m_outputFilename, std::ios::trunc);

    if (!strm.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
    }
    
    isx::DurationInSeconds baseTime{};
    switch (inParams.m_writeTimeRelativeTo)
    {
        case CellSetExporterParams::WriteTimeRelativeTo::FIRST_DATA_ITEM:
        {
            baseTime = srcs[0]->getTimingInfo().getStart().getSecsSinceEpoch();;
            break;
        }
        case CellSetExporterParams::WriteTimeRelativeTo::UNIX_EPOCH:
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
    strm << "Time(s), ";
    for (isize_t i = 0; i < numCells - 1; ++i)
    {
        strm << srcs[0]->getCellName(i) << ", ";
    }
    strm << srcs[0]->getCellName(numCells - 1) << "\n";
    
    // iterate over input cell sets
    for (auto & cs: srcs)
    {
        // loop over samples
        const isize_t numSamples = cs->getTimingInfo().getNumTimes();
        for (isize_t sample = 0; sample < numSamples; ++sample)
        {
            // write time point
            {
                auto tm = cs->getTimingInfo().convertIndexToMidTime(sample).getSecsSinceEpoch();
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
            cancelled = inCheckInCB(float(numLinesWritten) / float(numLinesTotal));
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

    if (cancelled)
    {
        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
