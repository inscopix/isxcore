#include "isxDecompression.h"

#include "isxCompressedMovieFile.h"
#include "isxDataSet.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"


namespace isx
{
const char *
DecompressParams::getFilenameSuffix()
{
    return "decompressed";
}

DataSet::Type
DecompressParams::getOutputDataSetType()
{
    return DataSet::Type::MOVIE;
}

AsyncTaskStatus
runDecompression(DecompressParams inParams, SpDecompressOutputParams_t inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    std::string outFileName =
        makeUniqueFilePath(
            inParams.m_outputDir + '/' +
                getBaseName(inParams.m_srcFilename) + '-' + DecompressParams::getFilenameSuffix() + ".isxd");

    /// Read header and setup the decoder
    CompressedMovieFile compressedMovie(inParams.m_srcFilename, outFileName);

    /// Decompress all frames
    isx::AsyncTaskStatus result = compressedMovie.readAllFrames(inCheckInCB);

    /// Set output file name
    inOutputParams->filename = outFileName;

    /// Remove file if cancel
    if (result == AsyncTaskStatus::CANCELLED)
    {
        if (isx::pathExists(outFileName))
        {
            std::remove(outFileName.c_str());
        }

    }

    return result;
}

} // namespace isx
