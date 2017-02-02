#ifndef ISX_EXPORT_H
#define ISX_EXPORT_H

#include "isxCoreFwd.h"
#include <string>


namespace isx
{
/// Export an image to TIFF
/// \param  inFileName  The filename for the output file.
/// \param inImage      The image to export.
void toTiff(const std::string & inFileName, const SpImage_t & inImage);

} // namespace isx

#endif // ISX_EXPORT_H
