#include "isxException.h"

namespace isx
{

Exception::Exception(const char* file, int line, const std::string& message)
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

const char*
Exception::getFileName() const
{
    return m_fileName.c_str();
}

int
Exception::getLine()
{
    return m_line;
}

ExceptionFileIO::ExceptionFileIO(const char* file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionFileIO::~ExceptionFileIO()
{
}

ExceptionDataIO::ExceptionDataIO(const char* file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionDataIO::~ExceptionDataIO()
{
}

ExceptionUserInput::ExceptionUserInput(const char* file, int line, const std::string& message)
    : Exception(file, line, message)
{
}

ExceptionUserInput::~ExceptionUserInput()
{
}

// Non API utilities.
namespace internal
{

void streamVarArgs_(std::ostringstream& strm)
{
}

} // namespace internal

} // namespace isx
