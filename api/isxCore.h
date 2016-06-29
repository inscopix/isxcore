#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>


namespace isx
{

    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    void CoreInitialize();
    bool CoreIsInitialized();
    void CoreShutdown();
}


#endif // def ISX_CORE_H
