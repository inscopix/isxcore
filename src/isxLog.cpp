#include "isxLog.h"

#if ISX_OS_WIN32
#define NOMINMAX
#include <windows.h>
#include "isxMutex.h"
#endif

#include <sstream>


namespace isx {

namespace {
    std::ostringstream sstm;
#if ISX_OS_WIN32
    isx::Mutex smutex;  //  protect stringstream above
#endif
}

namespace internal
{
std::ostringstream & getLogStream()
{
	return sstm;
}

#if ISX_OS_WIN32
void flushLogStream()
{
    isx::ScopedMutex(smutex, "flushLogStream");
    getLogStream().flush();
    OutputDebugString(getLogStream().str().c_str());
    getLogStream().str("");
}

void log_()
{
    std::ostringstream & sstm = isx::internal::getLogStream();
    sstm << "\n";
    isx::internal::flushLogStream();
}

#else

void flushLogStream()
{
	std::cout << std::flush;
}

void log_()
{
    std::cout << "\n";
    std::cout << std::flush;
}

#endif

} // namespace internal
} // namespace isx
