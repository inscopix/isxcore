#ifndef ISX_EXPORT_PNG_H
#define ISX_EXPORT_PNG_H

#include "isxCoreFwd.h"

namespace isx
{
    /// Export an image to PNG
    /// \param inFileName   The filename for the output file.
    /// \param inImage      The image to export.
    void toPNG(const std::string & inFileName, const SpImage_t & inImage);

} // namespace isx

#endif // ISX_EXPORT_PNG_H
