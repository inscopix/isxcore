#include "isxException.h"

#if ISX_OS_LINUX
#include <cstring>
#endif

namespace isx
{

Exception::Exception(const std::string& file, int line, const std::string& message)
    : m_msg(message)
    , m_fileName(file)
{
}

Exception::~Exception()
{
}

const char*
Exception::what() const noexcept
{
    return m_msg.c_str();
}

const std::string&
Exception::getFileName() const
{
    return m_fileName;
}

int
Exception::getLine()
{
    return m_line;
}

ExceptionFileIO::ExceptionFileIO(const std::string& file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionFileIO::~ExceptionFileIO()
{
}

ExceptionDataIO::ExceptionDataIO(const std::string& file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionDataIO::~ExceptionDataIO()
{
}

ExceptionUserInput::ExceptionUserInput(const std::string& file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionUserInput::~ExceptionUserInput()
{
}

ExceptionSeries::ExceptionSeries(const std::string& file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionSeries::~ExceptionSeries()
{
}

std::string getSystemErrorString()
{
    #if ISX_OS_MACOS || ISX_OS_LINUX
    return strerror(errno);
    #elif ISX_OS_WIN32
    char error[256];
    strerror_s(error, 100, errno);
    return error;
    #endif
}

// Non API utilities.
namespace internal
{

void streamVarArgs(std::ostringstream& strm)
{
}

} // namespace internal

} // namespace isx
