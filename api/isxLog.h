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

std::string baseName(const std::string& fileName);

} // namespace internal
} // namespace isx

#endif
