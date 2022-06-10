#ifndef ISX_SYNCHRONIZE_H
#define ISX_SYNCHRONIZE_H

#include "isxAsync.h"
#include <string>

namespace isx
{

/// Synchronize the epoch start times of files originating from the same paired and synced start-stop recording session.
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
AsyncTaskStatus synchronizeStartTimes(
    const std::string inRefFilename,
    const std::vector<std::string> inAlignFilenames
);

} // namespace isx

#endif // ISX_SYNCHRONIZE_H
