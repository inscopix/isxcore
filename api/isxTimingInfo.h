#ifndef ISX_TIMING_INFO_H
#define ISX_TIMING_INFO_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxTime.h"

#include <vector>

namespace isx
{

/// The timing info associated with temporal samples.
///
/// This class is used to store timing info about samples associated with
/// movies and traces. It also contains some utility methods to convert
/// times to sample indices so that samples can be retrieved with respect
/// to absolute time points.
class TimingInfo : public Object
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
    TimingInfo(const Time & start, const DurationInSeconds & step, isize_t numTimes);

    /// Get the start time of the samples.
    ///
    /// \return         The start time of the samples.
    Time getStart() const;

    /// Get the end time of the samples.
    ///
    /// \return         The end time of the samples.
    Time getEnd() const;

    /// Get the duration of one sample in seconds.
    ///
    /// \return         The duration of one sample in seconds.
    DurationInSeconds getStep() const;

    /// Get the number of time samples.
    ///
    /// \return         The number of time samples.
    isize_t getNumTimes() const;

    /// Get the duration of all samples in seconds.
    ///
    /// \return         The duration of all samples in seconds.
    DurationInSeconds getDuration() const;

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
    /// [0.2, 0.4). If the inTime is in [0.2, 0.4) then 1 will be returned.
    /// If the inTime is equal to 0.3, then 2 will be returned.
    ///
    /// If the time is earlier than the start time, then zero is returned.
    /// If the time is later than the end time, then the last index is
    /// returned.
    ///
    /// \param  inTime  The time to convert to an index.
    /// \return         The sample index closest to the given time.
    isize_t convertTimeToIndex(const Time & inTime) const;

    /// Converts an index to the absolute center time of its associated window.
    ///
    /// If the index exceeds the number of samples, it will effectively be
    /// clamped to be less than the number of samples.
    ///
    /// If the number of samples is 0, this will return the start time of the samples.
    ///
    /// \param  inIndex The index of the sample/window.
    /// \return         The absolute center of time of the window associated with inIndex.
    Time convertIndexToMidTime(isize_t inIndex) const;

    /// Converts an index to the absolute start time of its associated window.
    ///
    /// If the number of samples is 0, this will return the start time of the samples.
    ///
    /// \param inIndex index to convert, will be clamped to range of valid
    ///        indices in this movie.
    /// \return the start time of the window associated with inIndex
    Time convertIndexToStartTime(isize_t inIndex) const;

    /// \param  other   The other timing information with which to compare.
    /// \return         True if this is exactly equal to other, false otherwise.
    bool operator ==(const TimingInfo& other) const;

    // Overrides
    void serialize(std::ostream& strm) const override;


    /// Set the object valid/invalid
    /// \param inValid validity flag
    void setValid(bool inValid);

    /// \return whether this is a valid object or not
    ///
    bool isValid() const;

    /// \return a TimingInfo object initialized with default values
    /// \param inFrames
    static TimingInfo getDefault(isize_t inFrames);

private:

    /// The start time of the samples.
    Time m_start;

    /// The duration of one sample in seconds.
    DurationInSeconds m_step;

    /// The number of time samples.
    isize_t m_numTimes;

    /// Whether the object is valid or not
    bool m_isValid = false;

}; // class

/// type of container for multiple TimingInfo objects for MovieSeries, TraceSeries
using TimingInfos_t = std::vector<TimingInfo>;

} // namespace

#endif // ISX_TIMING_INFO_H
