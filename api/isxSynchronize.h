#ifndef ISX_SYNCHRONIZE_H
#define ISX_SYNCHRONIZE_H

#include "isxAsync.h"
#include "isxDataSet.h"
#include "isxExport.h"

#include <string>

namespace isx
{


/// The inputs, outputs and parameters of the export aligned timestamps operation.
struct ExportAlignedTimestampsParams
{
    /// Default
    ExportAlignedTimestampsParams()
    {
    }

    /// Convenience constructor to fill struct members in one-shot
    ///
    /// \param  inRefSeriesFilenames               The paths of the series to use as the timing reference to align with the other input files.
    ///                                            Timestamps are exported relative to the start time of the first file in the series.
    ///                                            These files can either be a .gpio file, .isxd movie or .isxb movie, otherwise the function will throw an error.
    ///                                            If the timing reference is a movie, the movie must contain frame timestamps, otherwise this function will throw an error.
    /// \param  inAlignSeriesFilenames             The paths of the series to align to the epoch start time of the input timing reference file.
    ///                                            These files can either be a .gpio file, .isxd movie or .isxb movie, otherwise the function will throw an error.
    ///                                            The movies must contain frame timestamps, otherwise this function will throw an error.
    /// \param inRefSeriesName                     The name of the timing reference series to use in the output csv
    /// \param inAlignSeriesNames                  The names of the align series to use in the output csv
    /// \param  inOutputFilename                   The path of the output csv file.
    /// \param inFormat                            The format of timestamps.
    ExportAlignedTimestampsParams(
        const std::vector<std::string> inRefSeriesFilenames,
        const std::vector<std::vector<std::string>> inAlignSeriesFilenames,
        const std::string inRefSeriesName,
        const std::vector<std::string> inAlignSeriesNames,
        const std::string inOutputFilename,
        const WriteTimeRelativeTo inFormat)
        : m_refSeriesFilenames(inRefSeriesFilenames)
        , m_alignSeriesFilenames(inAlignSeriesFilenames)
        , m_refSeriesName(inRefSeriesName)
        , m_alignSeriesNames(inAlignSeriesNames)
        , m_outputFilename(inOutputFilename)
        , m_format(inFormat)
    {
    }

    /// get filename suffix
    /// \return filename suffix for this algorithm
    ///
    static
    const char *
    getFilenameSuffix();

    /// get algorithm name
    /// \return the algorithm name
    ///
    static
    const char *
    getOpName();

    /// get name for output
    /// \return the name for the output
    ///
    static
    const char *
    getNameForOutput();

    /// get filename suffix
    /// \return filename suffix for this algorithm
    ///
    static
    DataSet::Type
    getOutputDataSetType();

    /// get a struct object of this type from a json-formatted string
    /// \param inStr the string to parse
    static
    ExportAlignedTimestampsParams
    fromString(const std::string & inStr);

    /// convert to string
    /// \return a JSON-formatted string with input parameters
    std::string
    toString() const;

    /// \return The input file paths.
    ///
    std::vector<std::string> getInputFilePaths() const;

    /// \return The output file paths.
    ///
    std::vector<std::string> getOutputFilePaths() const;

    /// The ref series filenames
    std::vector<std::string> m_refSeriesFilenames;

    /// The align series list filenames
    std::vector<std::vector<std::string>> m_alignSeriesFilenames;

    /// The ref series name
    std::string m_refSeriesName;

    /// The align series list filenames
    std::vector<std::string> m_alignSeriesNames;

    /// The output csv file
    std::string m_outputFilename;

    /// Export format for timestamps
    WriteTimeRelativeTo              m_format;
};


/// struct that defines the output parameters
struct ExportAlignedTimestampsOutputParams
{
    DataSet::Properties      m_dataSetProperties; ///< DataSet properties of output dataset, algo will set dmin and dmax
};
using SpExportAlignedTimestampsOutputParams_t = std::shared_ptr<ExportAlignedTimestampsOutputParams>;

/// Align the epoch start times of files originating from the same paired and synchronized start-stop recording session.
/// The epoch start time stored in the input align files are modified in-place
/// so that they are aligned relative to the epoch start time of the input timing reference file
/// For each input align file, the epoch start time is recomputed using the following formula:
///     align_epoch_start_ms = ref_epoch_start_ms + ((align_first_tsc_us - ref_first_tsc_us) / 1e3)
///
/// In the event that the first sample of an input align file is dropped, the tsc value of the first sample is inferred using the following formula:
///     align_first_tsc_us = align_first_valid_tsc_us - (align_first_valid_idx * align_sample_period_us)
///
/// \param inRefFilename The path of the file to use as the timing reference to align with the other input files.
/// This can be either a .gpio file, .isxd movie, or .isxb movie, otherwise the function will throw an error.
/// If the timing reference is a movie, the movie must contain frame timestamps, otherwise this function will throw an error.
/// \param inAlignFilenames The path of the files to align to the epoch start time of the input timing reference file.
/// These files can either be an .isxd movie or .isxb movie, otherwise the function will throw an error.
/// The movies must contain frame timestamps, otherwise this function will throw an error.
///
AsyncTaskStatus alignStartTimes(
    const std::string inRefFilename,
    const std::vector<std::string> inAlignFilenames
);

/// Export timestamps from files which originate from the same paired and synced start-stop recording session to a .csv file.
/// Timestamps are aligned to a single start time which is defined as the start time of the specified timing reference file.
AsyncTaskStatus exportAlignedTimestamps(
    ExportAlignedTimestampsParams inParams,
    SpExportAlignedTimestampsOutputParams_t outParams,
    AsyncCheckInCB_t inCheckInCB
);

} // namespace isx

#endif // ISX_SYNCHRONIZE_H
