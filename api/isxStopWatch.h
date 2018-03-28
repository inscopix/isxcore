#ifndef ISX_STOP_WATCH_H
#define ISX_STOP_WATCH_H


#if ISX_OS_MACOS
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#elif ISX_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif ISX_OS_LINUX
#include <time.h>
#include <cstdint>
#endif

namespace isx
{

/// Class implementing a timer/clock that can be used to measure
/// amount of time used to execute chunks of code
class StopWatch
{
public:
    /// Constructor
    ///
    StopWatch();

    /// start this stop watch
    ///
    void
    start();

    /// stop this stop watch
    ///
    void
    stop();

    /// reset this stop watch
    ///
    void
    reset();

    /// get elapsed time in milli seconds
    /// \return elapsed time in milli seconds
    ///
    float
    getElapsedMs();

private:
    bool m_isRunning;
#if ISX_OS_MACOS
    uint64_t m_startTime;
    uint64_t m_elapsedTime;
#elif ISX_OS_WIN32
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_elapsedTime;
#elif ISX_OS_LINUX
    struct timespec m_startTime;
    uint64_t m_elapsedNanoSeconds;
#endif
};


/// Class implementing a scoped stop watch
/// can be used like this
///
///    float timeElapsed = 0.f;
///    {
///        ScopedStopWatch sw(&timeElapsed);
///        doSomethingTimeCritical();
///    }
///    log("doSomethingTimeCritical took ", timeElapsed, "milliseconds.");
///
class ScopedStopWatch
{
public:
    /// Constructor.
    ///
    ScopedStopWatch(float * pElapsed);

    /// Destructor.
    ///
    ~ScopedStopWatch();

private:
    float * m_pElapsed;
    StopWatch m_stopWatch;
};

} // namespace isx

#endif // def ISX_STOP_WATCH_H
