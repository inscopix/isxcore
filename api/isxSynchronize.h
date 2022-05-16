#ifndef ISX_SYNCHRONIZE_H
#define ISX_SYNCHRONIZE_H

#include "isxAsync.h"
#include <string>

namespace isx
{

/// Synchronize the epoch start times of paired miniscope and behavior recordings using a reference miniscope gpio file.
/// The epoch start time stored in the .isxd and .isxb files are modified in-place so that they are aligned with the reference gpio file epoch start time.
/// For each movie, the epoch start time is recomputed using the following formula:
///     movie_epoch_start_ms = gpio_epoch_start_ms + ((movie_first_tsc_us - gpio_first_tsc_us) / 1e3)
///
/// In the event that the first frame of either movie is dropped, the tsc value of the first frame is inferred using the following formula:
///     movie_first_tsc_us = movie_first_valid_tsc_us - (movie_first_valid_idx * sample_period_us)
///
/// \param inGpioFilename The path of the reference gpio file from the miniscope system to use for synchronization.
/// \param inIsxdFilename The path of the isxd file from the miniscope system to modify in-place.
/// \param inIsxbFilename The path of the isxb file from the behavior system to modify in-place.
///
AsyncTaskStatus synchronizeStartTimes(
    const std::string inGpioFilename,
    const std::string inIsxdFilename,
    const std::string inIsxbFilename
);

} // namespace isx

#endif // ISX_SYNCHRONIZE_H
