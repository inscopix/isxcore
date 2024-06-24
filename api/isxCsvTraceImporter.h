#ifndef ISX_CSV_TRACE_IMPORTER_H
#define ISX_CSV_TRACE_IMPORTER_H

#include "isxCellSet.h"
#include "isxAsyncTaskHandle.h"

#include <set>

namespace isx
{

/// These parameters control the import of traces stored in a CSV file.
/// An output file will be created to store the imported values.
/// All time stamps are imported with microsecond precision and other
/// values are stored as floats.
/// Time stamps in the input file must be located in one column and
/// must be strictly increasing.
/// All row/column indices are 0-based.
/// If any channels have the same names, the import will fail.
struct CsvTraceImporterParams
{

    /// Default constructor
    ///
    CsvTraceImporterParams();

    /// \return Operation name displayed to the user.
    static std::string getOpName();

    /// contruct a CsvTraceImporterParams object from a json-formatted string
    /// (usually created with the toString() method)
    /// \param inStr the string to convert
    static 
    CsvTraceImporterParams 
    fromString(const std::string & inStr);

    /// \return a JSON-formatted string with input parameters
    ///
    std::string toString() const;

    /// \param  outMessage  The reason why the columns are invalid.
    /// \return             True if the columns are valid, false otherwise.
    bool checkColsToImport(std::string & outMessage) const;

    /// \return The input file paths.
    ///
    std::vector<std::string> getInputFilePaths() const;

    /// \return The output file paths.
    ///
    std::vector<std::string> getOutputFilePaths() const;

    std::string m_inputFile;            ///< The path of the input CSV file.
    std::string m_outputFile;           ///< The path of the output .isxd file.
    size_t m_startRow = 1;              ///< The row where the numerical data starts (i.e. not including the title row).
    std::set<size_t> m_colsToImport;    ///< The columns to import. If empty, import all of them.
    size_t m_titleRow = 0;              ///< The index of the title row that includes channel names.
    size_t m_timeCol = 0;               ///< The index of the time column.
    Time m_startTime;                   ///< The start time of the first sample.
    DurationInSeconds m_timeUnit = DurationInSeconds(1, 1);       ///< The duration of a unit of time in the time column.

}; // struct CsvTraceImporterParams

/// There are no output parameters for importing CSV traces.
struct CsvTraceImporterOutputParams
{
};

/// Runs CsvTraceImporter
/// \param inParams parameters for this CellSet export
/// \param inOutputParams a shared pointer for output parameters
/// \param inCheckInCB check-in callback function that is periodically invoked with progress and to tell algo whether to cancel / abort
AsyncTaskStatus runCsvTraceImporter(
        CsvTraceImporterParams inParams,
        std::shared_ptr<CsvTraceImporterOutputParams> inOutputParams,
        AsyncCheckInCB_t inCheckInCB);

/// Convert a Microsoft Excel index to a 0-based numerical index.
/// The Excel index can be numerical (starting at "1") or alphabetical
/// (starting at "A").
///
/// \param  inIndex     The Excel index (e.g. "1", "23", "A", "K", "BF").
/// \return             The equivalent numerical index.
size_t convertExcelIndexToIndex(const std::string & inIndex);

/// Convert many Excel column letters separated by commas to
/// numerical indices.
///
/// \param  inIndices   The string containing many Excel indices (e.g. "A, F, BA").
/// \return             The set of numerical indices.
/// \sa     convertExcelIndexToIndex
std::set<size_t> convertExcelIndicesToIndices(const std::string & inIndices);

} // namespace isx

#endif // ISX_CSV_TRACE_IMPORTER_H
