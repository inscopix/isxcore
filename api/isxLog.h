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
#define ISX_LOG_ERROR(...) ISX_LOG_INTERNAL(__VA_ARGS__, "\n")
#else
#define ISX_LOG_INFO(...) ISX_LOG_INTERNAL(__VA_ARGS__, "\n")
#define ISX_LOG_WARNING(...) ISX_LOG_INTERNAL(__VA_ARGS__, "\n")
#define ISX_LOG_ERROR(...) ISX_LOG_INTERNAL(__VA_ARGS__, "\n")
#endif

void ISX_LOG_INTERNAL() {}

template<typename First, typename ...Rest>
void ISX_LOG_INTERNAL(First && first, Rest && ...rest)
{
    std::cout << std::forward<First>(first);
    ISX_LOG_INTERNAL(std::forward<Rest>(rest)...);
}

#if 0
#if ISX_OS_WIN32
    #define ISX_LOG_INTERNAL(MSG)\
    { \
        std::ostringstream & sstm = isx::internal::getLogStream(); \
        sstm << ##MSG## ; \
        sstm << "\n" ; \
        isx::internal::flushLogStream(); \
    }
#else
    #define ISX_LOG_INTERNAL(MSG)\
    { \
        ISX_LOG_CONCAT_WRAP_INTERNAL(std::cout<<,MSG); \
        std::cout << "\n" ; \
        isx::internal::flushLogStream(); \
    }
    #define ISX_LOG_CONCAT_WRAP_INTERNAL(A,B) A ## B
#endif
#endif

#define ISX_LOG_INTERNAL_DETAILED(MSG)\
    QFileInfo fileInfo(__FILE__); \
    std::cout   << fileInfo.fileName().toStdString() << " : " \
                << __LINE__ << " : " \
                << isx::Time::now()->toString() << " : " \
                << std::this_thread::get_id() << " " \
                << MSG << std::endl;

#endif // ISX_LOG_H
