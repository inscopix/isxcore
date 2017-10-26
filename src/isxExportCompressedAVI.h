#ifndef ISX_EXPORT_COMPRESSED_AVI_H
#define ISX_EXPORT_COMPRESSED_AVI_H

#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"

#include <string>
#include <vector>

namespace isx
{

/// Export a movie to AVI
/// \param inFileName       The filename for the output file.
/// \param inMovies         The set of movies to export.
/// \param inMaxFrameIndex  The frames number in one movie part.
/// \param inCheckInCB      check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort.
bool toCompressedAVI(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const isize_t& inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB);

} // namespace isx

#endif // ISX_EXPORT_COMPRESSED_AVI_H
