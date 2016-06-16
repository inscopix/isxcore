#ifndef ISX_ASSERT_H
#define ISX_ASSERT_H

#include <assert.h>

#include "isxLog.h"

#ifdef NDEBUG
#define ISX_ASSERT(COND, ...)\
    if (!(COND))\
    {\
        ISX_LOG_ERROR(__VA_ARGS__);\
        std::string file = isx::internal::baseName(__FILE__);\
        ISX_LOG_ERROR(file, ":", __LINE__, ": Assertion `", #COND, "' failed.");\
    }\
    assert(COND)
#else
#define ISX_ASSERT(COND, ...)\
    if (!(COND))\
    {\
        ISX_LOG_ERROR(__VA_ARGS__);\
    }\
    assert(COND)
#endif

#endif // ISX_ASSERT_H
