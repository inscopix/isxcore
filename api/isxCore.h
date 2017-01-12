#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>
#include <vector>

#include <ostream>

namespace isx
{
    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    /// \cond doxygen chokes on enum class inside of namespace
    /// The data types of values.
    enum class DataType
    {
        U16 = 0,
        F32,
        U8,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// Get the size of a data type in bytes.
    ///
    /// \param  inDataType  The data type.
    /// \return             The size of the given data type in bytes.
    isize_t getDataTypeSizeInBytes(DataType inDataType);

    /// Overload of ostream << operator for DataType.
    ///
    /// \param   inStream   The output stream to which to print.
    /// \param   inDataType The data type to output.
    /// \return             The modified output stream.
    std::ostream & operator<<(
            std::ostream & inStream,
            DataType inDataType);

    void CoreInitialize(const std::string & inLogFileName = std::string());
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
