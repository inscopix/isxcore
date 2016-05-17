#ifndef ISX_LOG_H
#define ISX_LOG_H

#include "isxTime.h"
#include <QFileInfo>
#include <iostream>
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
#define ISX_LOG_INFO(MSG)
#define ISX_LOG_WARNING(MSG)
#define ISX_LOG_ERROR(MSG) ISX_LOG_INTERNAL(MSG)
#else
#define ISX_LOG_INFO(MSG) ISX_LOG_INTERNAL(MSG)
#define ISX_LOG_WARNING(MSG) ISX_LOG_INTERNAL(MSG)
#define ISX_LOG_ERROR(MSG) ISX_LOG_INTERNAL(MSG)
#endif

#define ISX_LOG_INTERNAL(MSG)\
{ \
    std::ostringstream & sstm = isx::internal::getLogStream(); \
	sstm << ##MSG## ; \
	sstm << "\n" ; \
	isx::internal::flushLogStream(); \
}

#define ISX_LOG_INTERNAL_DETAILED(MSG)\
    QFileInfo fileInfo(__FILE__); \
    std::cout   << fileInfo.fileName().toStdString() << " : " \
                << __LINE__ << " : " \
                << isx::Time::now()->toString() << " : " \
                << std::this_thread::get_id() << " " \
                << MSG << std::endl;

#endif // ISX_LOG_H
