#ifndef ISX_EXPORT_H
#define ISX_EXPORT_H

#include "isxCoreFwd.h"
#include <string>


namespace isx
{

/// \cond doxygen chokes on enum class inside of namespace
/// Enum to select how to write time stamps in exported file
enum class WriteTimeRelativeTo
{
    FIRST_DATA_ITEM,    ///< write time stamps relative to start time of data set
    UNIX_EPOCH          ///< write time stamps relative to unix epoch
};
/// \endcond doxygen chokes on enum class inside of namespace

/// Export an image to TIFF
/// \param  inFileName  The filename for the output file.
/// \param inImage      The image to export.
void toTiff(const std::string & inFileName, const SpImage_t & inImage);

} // namespace isx

#endif // ISX_EXPORT_H
