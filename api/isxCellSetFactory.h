#ifndef ISX_CELLSET_FACTORY_H
#define ISX_CELLSET_FACTORY_H

#include "isxCellSet.h"
#include "isxCoreFwd.h"

#include <string>
#include <vector>

namespace isx
{
/// Write a new mosaic cell set to a file.
///
/// This creates a new cell set in a file. If the file already exists, this will fail.
///
/// \param  inFileName      The name of the cell set file.
/// \param  inTimingInfo    The timing information of the cell set.
/// \param  inSpacingInfo   The spacing information of the cell set.
/// \param  inIsRoiSet      True if this came from drawing ROIs, false otherwise.
/// \return                 The mosaic cell set created.
///
/// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
SpCellSet_t writeCellSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const bool inIsRoiSet = false);

/// Read an existing cell set from a file.
///
/// If the extension is not recognized, this fails.
///
/// \param  inFileName      The name of the cell set file to read.
/// \param  enableWrite     Set to true to open in read-write mode
/// \return                 The imported cell set.
///
/// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
/// \throw  isx::ExceptionDataIO    If parsing the cell set file fails or
///                                 if the extension is not recognized.
SpCellSet_t readCellSet(const std::string & inFileName, bool enableWrite = false);

/// Read an existing series of cell sets from a vector of files.
/// If the extension is not recognizedfor ane of the filenames, this fails.
///
/// \param  inFileNames     A vector containing the names of the cell set files to read.
/// \param  enableWrite     Set to true to open in read-write mode
/// \return                 The imported cell set.
///
/// \throw  isx::ExceptionFileIO    If reading of any of the cell set files fails.
/// \throw  isx::ExceptionDataIO    If parsing for any the cell set files fails or
///                                 if their extension is not recognized.
SpCellSet_t readCellSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite = false);
}

#endif // ISX_CELLSET_FACTORY_H
