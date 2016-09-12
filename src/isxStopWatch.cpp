#include "isxStopWatch.h"

namespace isx
{

#if ISX_OS_MACOS
///////////////////////////////////////////////////////////////////////////////////////////
//
// MacOS implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
StopWatch::StopWatch()
: m_isRunning(false)
, m_startTime(0)
, m_elapsedTime(0) {}

void
StopWatch::start()
{
    m_startTime = mach_absolute_time();
    m_isRunning = true;
}
void
StopWatch::stop()
{
    if (m_isRunning)
    {
        uint64_t stopTime = mach_absolute_time();
        m_elapsedTime += stopTime - m_startTime;
        m_isRunning = false;
    }
}

void
StopWatch::reset()
{
    if (!m_isRunning)
    {
        m_elapsedTime = 0;
    }
}

float
StopWatch::getElapsedMs()
{
    static mach_timebase_info_data_t sTimebaseInfo;
    if (sTimebaseInfo.denom == 0)
    {
        mach_timebase_info(&sTimebaseInfo);
    }
    uint64_t elapsed_ns = m_elapsedTime * sTimebaseInfo.numer / sTimebaseInfo.denom;
    double elapsed_ms = double(elapsed_ns) / 1000.0 / 1000.0;

    return elapsed_ms;
}

#elif ISX_OS_WIN32
///////////////////////////////////////////////////////////////////////////////////////////
//
// Windows implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
StopWatch::StopWatch()
: m_isRunning(false)
{
    m_startTime.QuadPart = 0;
    m_elapsedTime.QuadPart = 0;
}

void
StopWatch::start()
{
    QueryPerformanceCounter(&m_startTime);
    m_isRunning = true;
}

void
StopWatch::stop()
{
    if (m_isRunning)
    {
        LARGE_INTEGER stopTime;
        QueryPerformanceCounter(&stopTime);
        m_elapsedTime.QuadPart += stopTime.QuadPart - m_startTime.QuadPart;
        m_isRunning = false;
    }
}

void
StopWatch::reset()
{
    if (!m_isRunning)
    {
        m_elapsedTime.QuadPart = 0;
    }
}

float
StopWatch::getElapsedMs()
{
    static LARGE_INTEGER oneThousand;
    static LARGE_INTEGER sFrequency;
    if (sFrequency.QuadPart == 0)
    {
        QueryPerformanceFrequency(&sFrequency);
        oneThousand.LowPart = 1000;
    }
    LARGE_INTEGER elapsed_1000_ticks;
    elapsed_1000_ticks.QuadPart = m_elapsedTime.QuadPart * oneThousand.QuadPart; // division by frequency results in seconds, we want miliseconds
    double elapsed_ms = double(elapsed_1000_ticks.QuadPart) / double(sFrequency.QuadPart);

    return float(elapsed_ms);
}

#elif ISX_OS_LINUX
///////////////////////////////////////////////////////////////////////////////////////////
//
// Linux implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
StopWatch::StopWatch()
    : m_isRunning(false)
    , m_elapsedNanoSeconds(0)
{
    m_startTime = {0};
}

void
StopWatch::start()
{
    clock_gettime(CLOCK_MONOTONIC, &m_startTime);
    m_isRunning = true;
}

void
StopWatch::stop()
{
    static const uint64_t sSecondsPerNanoSecond = 1000 * 1000 * 1000;
    if (m_isRunning)
    {
        struct timespec stopTime = {0};
        clock_gettime(CLOCK_MONOTONIC, &stopTime);
        m_elapsedNanoSeconds += sSecondsPerNanoSecond * (stopTime.tv_sec - m_startTime.tv_sec)
            + stopTime.tv_nsec - m_startTime.tv_nsec;
        m_isRunning = false;
    }
}

void
StopWatch::reset()
{
    if (!m_isRunning)
    {
        elapsedNanoSeconds_ = {0};
    }
}

float
StopWatch::getElapsedMs()
{
    static const double sMilliSecondsPerNanoSecond = 1000.0 * 1000.0;
    double elapsed_ms = double(m_elapsedNanoSeconds) / sMilliSecondsPerNanoSecond;

    return elapsed_ms;
}

#endif

ScopedStopWatch::ScopedStopWatch(float * pElapsed)
: m_pElapsed(pElapsed)
{
    m_stopWatch.start();
}

ScopedStopWatch::~ScopedStopWatch()
{
    m_stopWatch.stop();
    *m_pElapsed = m_stopWatch.getElapsedMs();
}

} // namespace isx
