#include "isxCellSetFactory.h"
#include "isxCellSetSeries.h"
#include "isxAsync.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxSeriesUtils.h"

#include <algorithm>
#include <cstring>
#include <cmath>

namespace isx
{
    CellSetSeries::CellSetSeries()
    {

    }

    CellSetSeries::CellSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite)
        : m_valid(false)
    {
        ISX_ASSERT(inFileNames.size() > 0);
        if (inFileNames.size() == 0)
        {
            return;
        }

        for(const auto &fn : inFileNames)
        {
            m_cellSets.emplace_back(readCellSet(fn, enableWrite));
        }

        std::sort(m_cellSets.begin(), m_cellSets.end(), [](SpCellSet_t a, SpCellSet_t b)
        {
            return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
        });

        // cell sets are sorted by start time now, check if they meet requirements
        std::string errorMessage;
        for (isize_t i = 1; i < m_cellSets.size(); ++i)
        {
            if (!checkNewMemberOfSeries({m_cellSets[i - 1]}, m_cellSets[i], errorMessage))
            {
                ISX_THROW(ExceptionSeries, errorMessage);
            }
        }

        m_gaplessTimingInfo = makeGaplessTimingInfo(getTimingInfosForSeries());

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

    void
    CellSetSeries::closeForWriting()
    {
        for (auto & c: m_cellSets)
        {
            c->closeForWriting();
        }
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
        return m_gaplessTimingInfo;
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

    SpFTrace_t 
    CellSetSeries::getTrace(isize_t inIndex) 
    {
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_gaplessTimingInfo);
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
        std::weak_ptr<CellSet> weakThis = shared_from_this();

        AsyncTaskResult<SpFTrace_t> asyncTaskResult;
        asyncTaskResult.setValue(std::make_shared<FTrace_t>(m_gaplessTimingInfo));

        isize_t counter = 0;
        bool isLast = false;
        isize_t offset = 0;

        for (const auto &cs : m_cellSets)
        {
            isLast = (counter == (m_cellSets.size() - 1));

            CellSetGetTraceCB_t finishedCB =
                [weakThis, &asyncTaskResult, offset, isLast, inCallback] (AsyncTaskResult<SpFTrace_t> inAsyncTaskResult)
            {
                auto sharedThis = weakThis.lock();
                if (!sharedThis)
                {
                    return;
                }

                if (inAsyncTaskResult.getException())
                {
                    asyncTaskResult.setException(inAsyncTaskResult.getException());
                }
                else
                {
                    // only continue copying if previous segments didn't throw
                    if (!asyncTaskResult.getException())
                    {
                        auto traceSegment = inAsyncTaskResult.get();
                        auto traceSeries = asyncTaskResult.get();
                        isize_t numTimes = traceSegment->getTimingInfo().getNumTimes();
                        isize_t numBytes = numTimes * sizeof(float);
                        float * vals = traceSeries->getValues();
                        vals += offset;
                        memcpy((char *)vals, (char *)traceSegment->getValues(), numBytes);
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
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
            const SpImage_t & inImage,
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

    CellSet::CellStatus 
    CellSetSeries::getCellStatus(isize_t inIndex)
    {
        return m_cellSets[0]->getCellStatus(inIndex);
    }

    Color
    CellSetSeries::getCellColor(isize_t inIndex)
    {
        return m_cellSets[0]->getCellColor(inIndex);
    }

    std::string
    CellSetSeries::getCellStatusString(isize_t inIndex)
    {
        return m_cellSets[0]->getCellStatusString(inIndex);
    }

    void 
    CellSetSeries::setCellStatus(isize_t inIndex, CellSet::CellStatus inStatus)
    {
        for(const auto &cs : m_cellSets)
        {
            cs->setCellStatus(inIndex, inStatus);
        }
    }

    void
    CellSetSeries::setCellColor(isize_t inIndex, const Color& inColor)
    {
        for (const auto &cs : m_cellSets)
        {
            cs->setCellColor(inIndex, inColor);
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

    std::vector<bool> 
    CellSetSeries::getCellActivity(isize_t inIndex) const
    {
        std::vector<bool> activity;
        for(const auto &cs : m_cellSets)
        {
            std::vector<bool> segment_act = cs->getCellActivity(inIndex);
            activity.push_back(segment_act.front());
        }
        return activity;
    }

    void 
    CellSetSeries::setCellActive(isize_t inIndex, const std::vector<bool> & inActive)
    {
        if (inActive.size() != 1)
        {
            ISX_ASSERT(inActive.size() == m_cellSets.size());
            for(isize_t i(0); i < m_cellSets.size(); ++i)
            {
                m_cellSets[i]->setCellActive(inIndex, {inActive.at(i)});
            }
        }
        else
        {
            for(const auto &cs : m_cellSets)
            {
                cs->setCellActive(inIndex, inActive);
            }
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

    bool
    CellSetSeries::isRoiSet() const
    {
        for (const auto & cs : m_cellSets)
        {
            if (!cs->isRoiSet())
            {
                return false;
            }
        }
        return m_cellSets.size() > 0;
    }
}
