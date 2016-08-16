#ifndef ISX_LOG_H
#define ISX_LOG_H

#include <iostream>
#include <sstream>
#include <utility>

#if ISX_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

/// \def ISX_LOG_DEBUG(...)
///
/// Logs the arguments as strings in a debug message.
#ifdef NDEBUG
#define ISX_LOG_DEBUG(...)
#else
#define ISX_LOG_DEBUG(...) isx::internal::log_(__VA_ARGS__)
#endif

/// \def ISX_LOG_INFO(...)
///
/// Logs the arguments as strings in an info message.
#define ISX_LOG_INFO(...) isx::internal::log_(__VA_ARGS__)

/// \def ISX_LOG_WARNING(...)
///
/// Logs the arguments as strings in a warning message.
#define ISX_LOG_WARNING(...) isx::internal::log_(__VA_ARGS__)

/// \def ISX_LOG_ERROR(...)
///
/// Logs the arguments as strings in an error message.
#define ISX_LOG_ERROR(...) isx::internal::log_(__VA_ARGS__)

// DO NOT USE THESE FUNCTIONS DIRECTLY! USE MACROS ABOVE INSTEAD!
namespace isx
{

// Non API utilities.
namespace internal
{

/// Stopping condition for recursive streamVarArgs.
///
/// \param  strm        The stream to which to append.
void streamVarArgs(std::ostringstream& strm);

/// Appends variadic arguments to an output string stream.
///
/// \param  strm        The stream to which to append.
/// \param  first       The next argument to append.
/// \param  rest        The rest of the arguments to append.
template<typename First, typename ...Rest>
void streamVarArgs(std::ostringstream& strm, First && first, Rest && ...rest)
{
    strm << std::forward<First>(first);
    streamVarArgs(strm, std::forward<Rest>(rest)...);
}

/// Converts variadic arguments to a string using an output string stream.
///
/// \param  rest        The arguments to convert to a string.
/// \return             The converted string.
template<typename ...Rest>
std::string varArgsToString(Rest && ...rest)
{
    std::ostringstream strm;
    streamVarArgs(strm, std::forward<Rest>(rest)...);
    return strm.str();
}

/// Appends variadic arguments to a platform specific output buffer.
///
/// \param  rest    The arguments to append to the output buffer.
template<typename ...Rest>
void log_(Rest && ...rest)
{
    std::string str = isx::internal::varArgsToString(std::forward<Rest>(rest)..., "\n");
#if ISX_OS_WIN32
    if (IsDebuggerPresent())
    {
        OutputDebugString(str.c_str());
    }
    else
    {
        std::cout << str;
        std::cout << std::flush;
    }
#else
    std::cout << str;
    std::cout << std::flush;
#endif
}

/// Strips the leading directory name from a file name.
///
/// \param  fileName    The full file name including directory name.
/// \return             The base name with the directory name removed.
std::string baseName(const std::string& fileName);

} // namespace internal

} // namespace isx

#endif
