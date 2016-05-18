#ifndef ISX_LOG_H
#define ISX_LOG_H

#include "isxTime.h"
#include <QFileInfo>
#include <iostream>
#include <utility>
#include <sstream>
#include <thread>

namespace isx
{
namespace internal
{
// INTERNAL USE ONLY! DO NOT USE THIS FUNCTION DIRECTLY! USE MACROS BELOW INSTEAD!
std::ostringstream & getLogStream();
void flushLogStream();
} // internal
} // namespace isx

#ifdef NDEBUG
#define ISX_LOG_INFO(...)
#define ISX_LOG_WARNING(...)
#define ISX_LOG_ERROR(...) isx::internal::ISX_LOG_INTERNAL(__VA_ARGS__)
#else
#define ISX_LOG_INFO(...) isx::internal::ISX_LOG_INTERNAL(__VA_ARGS__)
#define ISX_LOG_WARNING(...) isx::internal::ISX_LOG_INTERNAL(__VA_ARGS__)
#define ISX_LOG_ERROR(...) isx::internal::ISX_LOG_INTERNAL(__VA_ARGS__)
#endif

namespace isx
{
namespace internal
{
void ISX_LOG_INTERNAL();

template<typename First, typename ...Rest>
void ISX_LOG_INTERNAL(First && first, Rest && ...rest)
{
#if ISX_OS_WIN32
    std::ostringstream & sstm = isx::internal::getLogStream();
    sstm << std::forward<First>(first);
#else
    std::cout << std::forward<First>(first);
#endif
    ISX_LOG_INTERNAL(std::forward<Rest>(rest)...);
}
} // namespace internal
} // namespace isx
#define ISX_LOG_INTERNAL_DETAILED(MSG)\
    QFileInfo fileInfo(__FILE__); \
    std::cout   << fileInfo.fileName().toStdString() << " : " \
                << __LINE__ << " : " \
                << isx::Time::now()->toString() << " : " \
                << std::this_thread::get_id() << " " \
                << MSG << std::endl;

#endif // ISX_LOG_H
