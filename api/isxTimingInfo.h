#ifndef ISX_TIMINGINFO_H
#define ISX_TIMINGINFO_H

#include <cstdint>
#include "isxTime.h"

namespace isx
{

/// The timing info associated with temporal samples.
///
/// This class is used to store timing info about samples associated with
/// movies and traces. It also contains some utility methods to convert
/// times to sample indices so that samples can be retrieved with respect
/// to absolute time points.
class TimingInfo
{
public:

    /// Default constructor.
    ///
    TimingInfo();

    /// Fully specified constructor.
    ///
    /// \param start    The start time of the samples.
    /// \param step     The duration of one sample in seconds.
    /// \param numTimes The number of samples.
    TimingInfo(const isx::Time& start, const isx::Ratio& step, uint32_t numTimes);

    /// Get the start time of the samples.
    ///
    /// \return         The start time of the samples.
    isx::Time getStart() const;

    /// Get the end time of the samples.
    ///
    /// \return         The end time of the samples.
    isx::Time getEnd() const;

    /// Get the duration of one sample in seconds.
    ///
    /// \return         The duration of one sample in seconds.
    isx::Ratio getStep() const;

    /// Get the number of time samples.
    ///
    /// \return         The number of time samples.
    uint32_t getNumTimes() const;

    /// Get the duration of all samples in seconds.
    ///
    /// \return         The duration of all samples in seconds.
    isx::Ratio getDuration() const;

    /// Converts an absolute time to the corresponding index within this.
    ///
    /// This associates each sample with the center of its temporal bin.
    /// It returns the index with the center that is closest to the given
    /// time. If the time is equally close to two centers, then this
    /// returns the larger index.
    ///
    /// For example, consider temporal samples with the following bins.
    ///
    ///             Bin 0       Bin 1       Bin 2       Bin 3       Bin 4
    ///         +-----------+-----------+-----------+-----------+-------
    /// Time    0    0.1   0.2   0.3   0.4   0.5   0.6   0.7   0.8
    /// Index   0           1           2           3           4
    ///
    /// Index 1 represents temporal bin 1, which contains all times in
    /// [0.2, 0.3). If the inTime is in [0.2, 0.3) then 1 will be returned.
    /// If the inTime is equal to 0.3, then 2 will be returned.
    ///
    /// If the time is earlier than the start time, then zero is returned.
    /// If the time is later than the end time, then the last index is
    /// returned.
    ///
    /// \param      inTime  The time to convert to an index.
    /// \return             The sample index closest to the given time.
    uint32_t convertTimeToIndex(const isx::Time& inTime) const;

private:

    /// The start time of the samples.
    isx::Time m_start;

    /// The duration of one sample in seconds.
    isx::Ratio m_step;

    /// The number of time samples.
    uint32_t m_numTimes;

}; // class

} // namespace

#endif // ISX_TIMINGINFO_H
