#include "isxLog.h"

namespace isx {

std::ostream&
log()
{
    return isx::Log::instance()->ostream();
}

std::ostream&
info()
{
    return isx::Log::instance()->ostream() << "INFO: ";
}

std::ostream&
warning()
{
    return isx::Log::instance()->ostream() << "WARNING: ";
}

std::ostream&
error()
{
    return isx::Log::instance()->ostream() << "ERROR: ";
}

Log* Log::m_instance = 0;

Log::Log()
: m_strm(&std::cout)
{
}

Log::~Log()
{
}

isx::Log*
Log::instance()
{
    if (!m_instance)
        m_instance = new Log();
    return m_instance;
}

void
Log::ostream(std::ostream& strm)
{
    m_strm = &strm;
}

std::ostream&
Log::ostream() const
{
    return *m_strm;
}

} // namespace
