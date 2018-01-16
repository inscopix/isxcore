#ifndef ISX_EXPORT_TIFF_H
#define ISX_EXPORT_TIFF_H

#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"
#include "isxTrace.h"

#include <string>
#include <vector>

#include <tiffio.h>

namespace isx
{

/// class that defines TiffExporter that wraps TIFF file. TODO: we should have import-export logic in one place
class TiffExporter
{
public:
    /// constructor
    /// 
    /// \param inFileName   out file path
    /// \param inBigTiff    if true, write to the BigTIFF format, otherwise don't
    TiffExporter(const std::string & inFileName, const bool inBigTiff = false);

    /// destructor
    /// 
    ~TiffExporter();

    /// writes image to current Tiff directory
    /// 
    /// \param inImage       pointer to Image
    /// \param inZeroImage   boolean flag to write black (zero) TIFF using inImage parameters (e.g. width, height, number of channels etc.)
    void toTiffOut(const Image * inImage, const bool inZeroImage = false);

    /// switch to next TIFF Directory/Frame
    /// 
    void nextTiffDir();
private:
    TIFF * out;
    //int fd;
};

} // namespace isx

#endif // ISX_EXPORT_TIFF_H
