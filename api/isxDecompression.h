#ifndef ISX_DECOMPRESSION_H
#define ISX_DECOMPRESSION_H

#include <string>

#include "isxAsyncTaskHandle.h"
#include "isxDataSet.h"


namespace isx {

/// struct that defines DFF's input, output and parameters
struct DecompressParams
{
    /// convenience constructor to fill struct members in one-shot
    /// \param inOutputDir     output directory of the decompressed file
    /// \param inFilename      filename of the compressed file
    DecompressParams(
        const std::string & inOutputDir,
        const std::string & inFilename)
        : m_outputDir(inOutputDir)
        , m_srcFilename(inFilename)
    {}

    /// \return export operation name to display to user
    static
    std::string
    getOpName()
    {
        return "Decompression";
    }

    /// \return     A string representation of these parameters.
    ///
    std::string
    toString() const
    {
        return "null";
    }

    /// \return     The output folder paths.
    ///
    std::vector<std::string>
    getOutputFilePaths() const
    {
        return {m_outputDir};
    }

    /// \return The input file paths.
    ///
    std::vector<std::string>
    getInputFilePaths() const
    {
        return {m_srcFilename};
    }

    /// get filename suffix
    /// \return filename suffix for this algorithm
    ///
    static
    const char *
    getFilenameSuffix();

    /// get filename suffix
    /// \return filename suffix for this algorithm
    ///
    static
    DataSet::Type
    getOutputDataSetType();

    std::string   m_outputDir;     ///< directory of output file
    std::string   m_srcFilename;  ///< name of input file
};

/// Decompression output parameters
struct DecompressOutputParams
{
    std::string filename; ///< The paths of the output files.
};
using SpDecompressOutputParams_t = std::shared_ptr<DecompressOutputParams>;

/// Decompress input movie file and write to IDPS readable file to output
/// \param inParams          Decompress parameters
/// \param inOutputParams    Decompress output parameters
/// \param inCheckInCB       callback to invoke periodically
AsyncTaskStatus runDecompression(DecompressParams inParams, SpDecompressOutputParams_t inOutputParams, AsyncCheckInCB_t inCheckInCB);
}


#endif //ISX_DECOMPRESSION_H
