#include "isxCellSetFactory.h"
#include "isxCellSetSeries.h"
#include "isxAsync.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"

#include <algorithm>
#include <cstring>

namespace isx
{
    CellSetSeries::CellSetSeries()
    {

    }

    CellSetSeries::CellSetSeries(const std::vector<std::string> & inFileNames)
        : m_valid(false)
    {
        ISX_ASSERT(inFileNames.size() > 0);
        if (inFileNames.size() == 0)
        {
            return;
        }

        for(const auto &fn : inFileNames)
        {
            m_cellSets.emplace_back(readCellSet(fn));
        }

        std::sort(m_cellSets.begin(), m_cellSets.end(), [](SpCellSet_t a, SpCellSet_t b)
        {
            return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
        });

        // cell sets are sorted by start time now, check if they meet requirements
        const auto & refSi = m_cellSets[0]->getSpacingInfo();
        const isize_t refNumCells = m_cellSets[0]->getNumCells();
        std::string errorMessage;
        for (isize_t i = 1; i < m_cellSets.size(); ++i)
        {
            const auto & cs = m_cellSets[i];

            if (cs->getNumCells() != refNumCells)
            {
                ISX_THROW(ExceptionSeries, "CellSetSeries with mismatching number of cells: ", cs->getFileName());
            }

            if (!Series::checkSpacingInfo(refSi, cs->getSpacingInfo(), errorMessage))
            {
                ISX_THROW(ExceptionSeries, errorMessage);
            }

            const auto & tip = m_cellSets[i-1]->getTimingInfo();
            const auto & tic = m_cellSets[i]->getTimingInfo();
            if (!Series::checkTimingInfo(tip, tic, errorMessage))
            {
                ISX_THROW(ExceptionSeries, errorMessage);
            }
        }

        Time start = m_cellSets[0]->getTimingInfo().getStart();
        DurationInSeconds step = m_cellSets[0]->getTimingInfo().getStep();
        Time end = m_cellSets.back()->getTimingInfo().getEnd();
        isize_t totalNumTimes = (isize_t)(DurationInSeconds(end - start).toDouble() / step.toDouble());
        m_timingInfo = TimingInfo(start, step, totalNumTimes);

        m_valid = true;
    }

    CellSetSeries::~CellSetSeries()
    {

    }

    bool 
    CellSetSeries::isValid() const
    {
        return m_valid;
    }

    std::string 
    CellSetSeries::getFileName() const 
    {   
        return "**CellSetSeries";        
    }

    const isize_t 
    CellSetSeries::getNumCells() 
    {
        return m_cellSets[0]->getNumCells();
    }

    isx::TimingInfo 
    CellSetSeries::getTimingInfo() const 
    {
        return m_timingInfo;
    }

    isx::TimingInfos_t 
    CellSetSeries::getTimingInfosForSeries() const 
    {
        TimingInfos_t tis;
        for(const auto &cs : m_cellSets)
        {
            tis.emplace_back(cs->getTimingInfo());
        }
        return tis;
    }

    isx::SpacingInfo 
    CellSetSeries::getSpacingInfo() const 
    {
        return m_cellSets[0]->getSpacingInfo();
    }

    TimingInfo 
    CellSetSeries::getGaplessTimingInfo()
    {
        isize_t totalNumTimes = 0;
        for (const auto &cs : m_cellSets)
        {
            totalNumTimes += cs->getTimingInfo().getNumTimes();
        }
        TimingInfo first = m_cellSets.front()->getTimingInfo();
        TimingInfo timingInfoGapless(first.getStart(), first.getStep(), totalNumTimes);
        return timingInfoGapless;
    }

    SpFTrace_t 
    CellSetSeries::getTrace(isize_t inIndex) 
    {
        TimingInfo timingInfoGapless = getGaplessTimingInfo();

        SpFTrace_t trace = std::make_shared<FTrace_t>(timingInfoGapless);
        float * v = trace->getValues();

        for (const auto &cs : m_cellSets)
        {
            SpFTrace_t partialTrace = cs->getTrace(inIndex);
            float * vPartial = partialTrace->getValues();
            isize_t numSamples = partialTrace->getTimingInfo().getNumTimes();
            memcpy((char *)v, (char *)vPartial, sizeof(float)*numSamples);
            v += numSamples;
        }
        return trace;  
    }

    void 
    CellSetSeries::getTraceAsync(isize_t inIndex, CellSetGetTraceCB_t inCallback) 
    {
        
        TimingInfo timingInfoGapless = getGaplessTimingInfo();

        std::weak_ptr<CellSet> weakThis = shared_from_this();

        SpFTrace_t trace = std::make_shared<FTrace_t>(timingInfoGapless);        

        isize_t counter = 0;
        bool isLast = false;
        isize_t offset = 0;

        for (const auto &cs : m_cellSets)
        {
            isLast = (counter == (m_cellSets.size() - 1));


            CellSetGetTraceCB_t finishedCB = [weakThis, &trace, offset, isLast, inCallback] (const SpFTrace_t & inTrace)
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }

                isize_t numTimes = inTrace->getTimingInfo().getNumTimes();
                isize_t numBytes = numTimes * sizeof(float);
                float * vals = trace->getValues();
                vals += offset;
                memcpy((char *)vals, (char *)inTrace->getValues(), numBytes);

                if(isLast)
                {
                    inCallback(trace);
                }
            };

            cs->getTraceAsync(inIndex, finishedCB);
            isize_t numSamples = cs->getTimingInfo().getNumTimes();
            offset += numSamples;
            ++counter;
            
        }     

    }

    SpImage_t 
    CellSetSeries::getImage(isize_t inIndex) 
    {
        return m_cellSets[0]->getImage(inIndex);
    }

    void 
    CellSetSeries::getImageAsync(isize_t inIndex, CellSetGetImageCB_t inCallback) 
    {
        return m_cellSets[0]->getImageAsync(inIndex, inCallback);
    }  

    void 
    CellSetSeries::writeImageAndTrace(
            isize_t inIndex,
            SpImage_t & inImage,
            SpFTrace_t & inTrace,
            const std::string & inName) 
    {
        // Don't do anything. Note: salpert 11/8/16 we need to decide 
        // what operations to allow on a cell set series (read/write)
        // and take appropriate actions to limit the API (maybe separate CellSet
        // into CellSet and WritableCellSet like we did for movies?)
        // This class will probably require some re-factoring
        ISX_ASSERT(false);
    } 

    bool 
    CellSetSeries::isCellValid(isize_t inIndex) 
    {
        return m_cellSets[0]->isCellValid(inIndex);
    }

    void 
    CellSetSeries::setCellValid(isize_t inIndex, bool inIsValid) 
    {
        for(const auto &cs : m_cellSets)
        {
            cs->setCellValid(inIndex, inIsValid);
        }
    }

    std::string 
    CellSetSeries::getCellName(isize_t inIndex) 
    {
        return m_cellSets[0]->getCellName(inIndex);
    }

    void 
    CellSetSeries::setCellName(isize_t inIndex, const std::string & inName) 
    {
        for(const auto &cs : m_cellSets)
        {
            cs->setCellName(inIndex, inName);
        }
    }

    void
    CellSetSeries::cancelPendingReads()
    {
        for (const auto &cs : m_cellSets)
        {
            cs->cancelPendingReads();
        }
    }



}
