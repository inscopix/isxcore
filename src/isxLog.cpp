#include "isxLog.h"

#if ISX_OS_WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <sstream>


namespace isx {

namespace {
    std::ostringstream sstm;
}

namespace internal
{
std::ostringstream & getLogStream()
{
	return sstm;
}

void flushLogStream()
{
#if ISX_OS_WIN32
    getLogStream().flush();
	OutputDebugString(getLogStream().str().c_str());
    getLogStream().str("");
#else
	std::cout << std::flush;
#endif
}

#if ISX_OS_WIN32
void ISX_LOG_INTERNAL()
{
    std::ostringstream & sstm = isx::internal::getLogStream();
    sstm << "\n";
    isx::internal::flushLogStream();
}
#else
void ISX_LOG_INTERNAL()
{
    std::cout << "\n";
    std::cout << std::flush;
}
#endif

} // namespace internal
} // namespace isx
