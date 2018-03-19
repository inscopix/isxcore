#include "isxEventsSeries.h"
#include "isxAsync.h"
#include "isxAsyncTaskHandle.h"
#include "isxDispatchQueue.h"
#include "isxSeries.h"
#include "isxException.h"
#include "isxSeriesUtils.h"

#include <algorithm>
#include <string.h>

namespace isx
{

const std::string EventsSeries::s_fileName = "**EventsSeries";

EventsSeries::EventsSeries()
{
}

EventsSeries::EventsSeries(const std::vector<std::string> & inFileNames)
{
    ISX_ASSERT(inFileNames.size() > 0);
    if (inFileNames.size() == 0)
    {
        return;
    }

    for (const auto & fn : inFileNames)
    {
        m_events.emplace_back(readEvents(fn));
    }

    std::sort(m_events.begin(), m_events.end(), [](SpEvents_t a, SpEvents_t b)
    {
        return a->getTimingInfo().getStart() < b->getTimingInfo().getStart();
    });

    // events are sorted by start time now, check if they meet requirements
    std::string errorMessage;
    for (isize_t i = 1; i < m_events.size(); ++i)
    {
        if (!checkNewMemberOfSeries({m_events[i - 1]}, m_events[i], errorMessage))
        {
            ISX_THROW(ExceptionSeries, errorMessage);
        }
    }

    m_gaplessTimingInfo = makeGaplessTimingInfo(getTimingInfosForSeries());

    m_valid = true;
}

EventsSeries::~EventsSeries()
{
}

bool
EventsSeries::isValid() const
{
    return m_valid;
}

const std::string &
EventsSeries::getFileName() const
{
    return s_fileName;
}

isize_t
EventsSeries::numberOfCells()
{
    return m_events[0]->numberOfCells();
}

const std::vector<std::string>
EventsSeries::getCellNamesList() const
{
    return m_events[0]->getCellNamesList();
}

SpLogicalTrace_t
EventsSeries::getLogicalData(const std::string & inCellName)
{
    SpLogicalTrace_t trace = std::make_shared<LogicalTrace>(m_gaplessTimingInfo, inCellName);
    for (const auto & e : m_events)
    {
        const SpLogicalTrace_t eTrace = e->getLogicalData(inCellName);
        if (eTrace != nullptr)
        {
            for (const auto & kv : eTrace->getValues())
            {
                trace->addValue(kv.first, kv.second);
            }
        }
    }
    return trace;
}

void
EventsSeries::getLogicalDataAsync(const std::string & inCellName, EventsGetLogicalDataCB_t inCallback)
{
    std::weak_ptr<Events> weakThis = shared_from_this();

    AsyncTaskResult<SpLogicalTrace_t> asyncTaskResult;
    asyncTaskResult.setValue(std::make_shared<LogicalTrace>(m_gaplessTimingInfo, inCellName));

    isize_t counter = 0;
    bool isLast = false;

    for (const auto & e : m_events)
    {
        isLast = (counter == (m_events.size() - 1));

        EventsGetLogicalDataCB_t finishedCB =
            [weakThis, &asyncTaskResult, isLast, inCallback] (AsyncTaskResult<SpLogicalTrace_t> inAsyncTaskResult)
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
                        const auto traceSegment = inAsyncTaskResult.get();
                        auto traceSeries = asyncTaskResult.get();
                        for (const auto & kv : traceSegment->getValues())
                        {
                            traceSeries->addValue(kv.first, kv.second);
                        }
                    }
                }

                if (isLast)
                {
                    inCallback(asyncTaskResult);
                }
            };

        e->getLogicalDataAsync(inCellName, finishedCB);
        ++counter;
    }
}

isx::TimingInfo
EventsSeries::getTimingInfo() const
{
    return m_gaplessTimingInfo;
}

isx::TimingInfos_t
EventsSeries::getTimingInfosForSeries() const
{
    TimingInfos_t tis;
    for (const auto & e : m_events)
    {
        tis.emplace_back(e->getTimingInfo());
    }
    return tis;
}

void
EventsSeries::cancelPendingReads()
{
    for (const auto & e : m_events)
    {
        e->cancelPendingReads();
    }
}

bool 
EventsSeries::hasMetrics() const 
{
    return m_events.front()->hasMetrics();
}

SpTraceMetrics_t 
EventsSeries::getTraceMetrics(isize_t inIndex) const 
{
    return m_events.front()->getTraceMetrics(inIndex);
}

void
EventsSeries::setTraceMetrics(isize_t inIndex, const SpTraceMetrics_t & inMetrics) 
{
    for (const auto & e : m_events)
    {
        e->setTraceMetrics(inIndex, inMetrics);
    }
}

} // namespace isx
