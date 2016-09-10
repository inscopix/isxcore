//
//  MyTimer.hpp
//  MyTimer
//
//  Created by Axel Schildan on 4/5/16.
//  Copyright © 2016 Axel Schildan. All rights reserved.
//
/// \cond doxygen skip this file

#ifndef ISX_PERFORMANCE_TIMER_H
#define ISX_PERFORMANCE_TIMER_H


#if ISX_OS_MACOS
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#elif ISX_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif ISX_OS_LINUX
// TODO: implement
#endif

namespace isx
{


#if ISX_OS_MACOS
///////////////////////////////////////////////////////////////////////////////////////////
//
// MacOS implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
class PerformanceTimer
{
public:
    PerformanceTimer()
    : isRunning(false)
    , startTime_(0)
    , elapsedTime_(0) {}
    void start()
    {
        startTime_ = mach_absolute_time();
        isRunning = true;
    }
    void stop()
    {
        if (isRunning)
        {
            uint64_t stopTime = mach_absolute_time();
            elapsedTime_ += stopTime - startTime_;
            isRunning = false;
        }
    }
    void reset()
    {
        if (!isRunning)
        {
            elapsedTime_ = 0;
        }
    }
    float getElapsed_ms()
    {
        static mach_timebase_info_data_t sTimebaseInfo;
        if (sTimebaseInfo.denom == 0)
        {
            mach_timebase_info(&sTimebaseInfo);
        }
        uint64_t elapsed_ns = elapsedTime_ * sTimebaseInfo.numer / sTimebaseInfo.denom;
        double elapsed_ms = double(elapsed_ns) / 1000.0 / 1000.0;

        return elapsed_ms;
    }
private:
    bool isRunning;
    uint64_t startTime_;
    uint64_t elapsedTime_;
};

#elif ISX_OS_WIN32
///////////////////////////////////////////////////////////////////////////////////////////
//
// Windows implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
class PerformanceTimer
{
public:
    PerformanceTimer()
    : isRunning(false)
    {
        startTime_.QuadPart = 0;
        elapsedTime_.QuadPart = 0;
    }
    void start()
    {
        QueryPerformanceCounter(&startTime_);
        isRunning = true;
    }
    void stop()
    {
        if (isRunning)
        {
            LARGE_INTEGER stopTime;
            QueryPerformanceCounter(&stopTime);
            elapsedTime_.QuadPart += stopTime.QuadPart - startTime_.QuadPart;
            isRunning = false;
        }
    }
    void reset()
    {
        if (!isRunning)
        {
            elapsedTime_.QuadPart = 0;
        }
    }
    float getElapsed_ms()
    {
        static LARGE_INTEGER oneThousand;
        static LARGE_INTEGER sFrequency;
        if (sFrequency.QuadPart == 0)
        {
            QueryPerformanceFrequency(&sFrequency);
            oneThousand.LowPart = 1000;
        }
        LARGE_INTEGER elapsed_1000_ticks;
        elapsed_1000_ticks.QuadPart = elapsedTime_.QuadPart * oneThousand.QuadPart; // division by frequency results in seconds, we want miliseconds
        double elapsed_ms = double(elapsed_1000_ticks.QuadPart) / double(sFrequency.QuadPart);

        return elapsed_ms;
    }
private:
    bool isRunning;
    LARGE_INTEGER startTime_;
    LARGE_INTEGER elapsedTime_;
};


#elif ISX_OS_LINUX
///////////////////////////////////////////////////////////////////////////////////////////
//
// Linux implementation
//
///////////////////////////////////////////////////////////////////////////////////////////
// TODO: implement
class PerformanceTimer
{
public:
    PerformanceTimer() {}
//        : isRunning(false)
//        , startTime_(0)
//        , elapsedTime_(0) {}
    void start()
    {
//        startTime_ = mach_absolute_time();
//        isRunning = true;
    }
    void stop()
    {
//        if (isRunning)
//        {
//            uint64_t stopTime = mach_absolute_time();
//            elapsedTime_ += stopTime - startTime_;
//            isRunning = false;
//        }
    }
    void reset()
    {
//        if (!isRunning)
//        {
//            elapsedTime_ = 0;
//        }
    }
    float getElapsed_ms()
    {
//        static mach_timebase_info_data_t sTimebaseInfo;
//        if (sTimebaseInfo.denom == 0)
//        {
//            mach_timebase_info(&sTimebaseInfo);
//        }
//        uint64_t elapsed_ns = elapsedTime_ * sTimebaseInfo.numer / sTimebaseInfo.denom;
//        double elapsed_ms = double(elapsed_ns) / 1000.0 / 1000.0;
//
//        return elapsed_ms;
        return 0.f;
    }
private:
//    bool isRunning;
//    uint64_t startTime_;
//    uint64_t elapsedTime_;
};

#endif


class ScopedPerformanceTimer
{
public:
    ScopedPerformanceTimer(float * pElapsed)
    : pElapsed_(pElapsed)
    {
        t_.start();
    }
    
    ~ScopedPerformanceTimer()
    {
        t_.stop();
        *pElapsed_ = t_.getElapsed_ms();
    }
    
private:
    float * pElapsed_;
    PerformanceTimer t_;
};


} // namespace isx

#endif // def ISX_PERFORMANCE_TIMER_H
/// \endcond doxygen chokes on enum class inside of namespace
