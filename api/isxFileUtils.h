#ifndef ISX_FILE_UTILS_H
#define ISX_FILE_UTILS_H

#include <string>


namespace isx
{
    /// \cond doxygen chokes on enum class inside of namespace
    enum class Hdf5Mode : uint8_t
    {
        NONE    = 0x00,
        IMAGING = 0x01,
        GPIO    = 0x02
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    constexpr Hdf5Mode operator |( const Hdf5Mode lhs, const Hdf5Mode rhs )
    {
        return (Hdf5Mode)(uint8_t(lhs) | uint8_t(rhs));
    }

    constexpr bool operator &( const Hdf5Mode lhs, const Hdf5Mode rhs )
    {
        return (uint8_t(lhs) & uint8_t(rhs)) > 0;
    }

    inline Hdf5Mode& operator |= (Hdf5Mode& lhs, Hdf5Mode rhs)
    {
        lhs = (Hdf5Mode)(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
        return lhs;
    }


    Hdf5Mode peekHdf5Modality(const std::string & inFileName);
    
}

#endif // ISX_FILE_UTILS_H