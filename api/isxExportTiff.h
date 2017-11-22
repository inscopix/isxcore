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
    TiffExporter(const std::string & inFileName);

    /// destructor
    /// 
    ~TiffExporter();

    /// writes image to current Tiff directory
    /// 
    /// \param inImage   pointer to Image
    void toTiffOut(const Image * inImage);

    /// switch to next TIFF Directory/Frame
    /// 
    void nextTiffDir();
private:
    TIFF * out;
    //int fd;
};

/// Export an image to TIFF
/// \param inFileName  The filename for the output file.
/// \param inImage      The image to export.
void toTiff(const std::string & inFileName, const SpImage_t & inImage);

/// Export a movie to TIFF
/// \param inFileName       The filename for the output file.
/// \param inMovies         The set of movies to export.
/// \param inMaxFrameIndex  The frames number in one movie part.
/// \param inCheckInCB      check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort.
bool toTiff(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB);

} // namespace isx

#endif // ISX_EXPORT_TIFF_H
