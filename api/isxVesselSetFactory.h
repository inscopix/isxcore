#ifndef ISX_VESSEL_SET_FACTORY_H
#define ISX_VESSEL_SET_FACTORY_H

#include "isxVesselSet.h"
#include "isxCoreFwd.h"

#include <string>
#include <vector>

namespace isx
{
/// Write a new vessel set to a file.
///
/// This creates a new vessel set in a file. If the file already exists, this will fail.
///
/// \param  inFileName      The name of the vessel set file.
/// \param  inTimingInfo    The timing information of the vessel set.
/// \param  inSpacingInfo   The spacing information of the vessel set.
/// \param  inVesselSetType The type of vessel set (vessel diameter or rbc velocity).
/// \return                 The vessel set created.
///
/// \throw  isx::ExceptionFileIO    If writing the vessel set file fails.
SpVesselSet_t writeVesselSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const VesselSetType_t inVesselSetType);

/// Read an existing vessel set from a file.
///
/// If the extension is not recognized, this fails.
///
/// \param  inFileName      The name of the vessel set file to read.
/// \param  enableWrite     Set to true to open in read-write mode
/// \return                 The imported vessel set.
///
/// \throw  isx::ExceptionFileIO    If reading the vessel set file fails.
/// \throw  isx::ExceptionDataIO    If parsing the vessel set file fails or
///                                 if the extension is not recognized.
SpVesselSet_t readVesselSet(const std::string & inFileName, bool enableWrite = false);

/// Read an existing series of vessel sets from a vector of files.
/// If the extension is not recognized for any of the filenames, this fails.
///
/// \param  inFileNames     A vector containing the names of the vessel set files to read.
/// \param  enableWrite     Set to true to open in read-write mode
/// \return                 The imported vessel set.
///
/// \throw  isx::ExceptionFileIO    If reading of any of the vessel set files fails.
/// \throw  isx::ExceptionDataIO    If parsing for any the vessel set files fails or
///                                 if their extension is not recognized.
SpVesselSet_t readVesselSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite = false);
}

#endif // ISX_VESSEL_SET_FACTORY_H
