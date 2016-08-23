#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>
#include <vector>

namespace isx
{
    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    /// \cond doxygen chokes on enum class inside of namespace
    /// The data types of values.
    enum class DataType
    {
        U16 = 0,
        F32
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    void CoreInitialize();
    bool CoreIsInitialized();
    void CoreShutdown();

    int CoreVersionMajor();
    int CoreVersionMinor();
    int CoreVersionPatch();

    /// \return     The version numbers in a vector.
    ///
    std::vector<int> CoreVersionVector();

} // namespace isx

#endif // def ISX_CORE_H
