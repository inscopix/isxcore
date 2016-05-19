#ifndef ISX_LOG_H
#define ISX_LOG_H

#include <iostream>
#include <utility>
#include <sstream>

#ifdef NDEBUG
#define ISX_LOG_DEBUG(...)
#define ISX_LOG_INFO(...) isx::internal::log_(__VA_ARGS__)
#define ISX_LOG_WARNING(...) isx::internal::log_(__VA_ARGS__)
#define ISX_LOG_ERROR(...) isx::internal::log_(__VA_ARGS__)
#else
#define ISX_LOG_DEBUG(...) isx::internal::log_(__VA_ARGS__)
#define ISX_LOG_INFO(...) isx::internal::log_(__VA_ARGS__)
#define ISX_LOG_WARNING(...) isx::internal::log_(__VA_ARGS__)
#define ISX_LOG_ERROR(...) isx::internal::log_(__VA_ARGS__)
#endif

// DO NOT USE THESE FUNCTIONS DIRECTLY! USE MACROS ABOVE INSTEAD!
namespace isx
{
namespace internal
{
std::ostringstream & getLogStream();
void flushLogStream();
void log_();

template<typename First, typename ...Rest>
void log_(First && first, Rest && ...rest)
{
#if ISX_OS_WIN32
    std::ostringstream & sstm = isx::internal::getLogStream();
    sstm << std::forward<First>(first);
#else
    std::cout << std::forward<First>(first);
#endif
    log_(std::forward<Rest>(rest)...);
}
} // namespace internal0
} // namespace isx

#if 0
#include "isxTime.h"
#include <QFileInfo>
#include <thread>
#define ISX_LOG_INTERNAL_DETAILED(MSG)\
    QFileInfo fileInfo(__FILE__); \
    std::cout   << fileInfo.fileName().toStdString() << " : " \
                << __LINE__ << " : " \
                << isx::Time::now()->toString() << " : " \
                << std::this_thread::get_id() << " " \
                << MSG << std::endl;

#endif // ISX_LOG_H
#endif