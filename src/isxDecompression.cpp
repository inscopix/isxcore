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

    /// Check space, need information from the header and session from the file
    // Same as sufficientDiskSpace() from algo/isxOutputValidation since we can't access that function
    const isize_t compressedMovieSizeInBytes = compressedMovie.getDecompressedFileSize();
    std::string rootDir;
    long long availableNumBytes = availableNumberOfBytesOnVolume(inParams.m_outputDir, rootDir);
    bool partitionExists = availableNumBytes >= 0;
    ISX_ASSERT(partitionExists);

    if (isize_t(availableNumBytes) <= compressedMovieSizeInBytes)
    {
        const double availableGB = double(availableNumBytes) / std::pow(2, 30);
        const double requiredGB = double(compressedMovieSizeInBytes) / std::pow(2, 30);

        ISX_THROW(
            ExceptionUserInput,
            "You have insufficient disk space to write the output files.",
            "The outputs will require about ", requiredGB, " GB.",
            "We estimate you only have about ", availableGB, " GB in the destination partition. ");
    }

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
