#ifndef ISX_TIMING_INFO_H
#define ISX_TIMING_INFO_H

#include "isxObject.h"
#include "isxCore.h"
#include "isxTime.h"
#include "isxIndexRange.h"

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
    /// \param start            The start time of the samples.
    /// \param step             The duration of one sample in seconds.
    /// \param numTimes         The number of samples.
    /// \param droppedFrames    A vector containing frame numbers that were dropped
    /// \param cropped          A vector containing index ranges that have been cropped.
    TimingInfo(
            const Time & start,
            const DurationInSeconds & step,
            isize_t numTimes,
            const std::vector<isize_t> & droppedFrames = std::vector<isize_t>(),
            const IndexRanges_t & cropped = {});

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
    
    /// Computes the start time of the last sample in the data set.
    /// This is different from getEnd() which returns the end time of the last sample.
    ///
    /// \return the start time of the last sample in the data set.
    Time getLastStartTime() const;

    /// \return     True if this overlaps in time with another timing info.
    ///
    bool overlapsWith(const TimingInfo & inOther) const;

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

    /// \return the vector of dropped frame numbers
    ///
    const std::vector<isize_t> & getDroppedFrames() const;

    /// \return the number of dropped frames
    /// Provided for convenience. Same as getDroppedFrames().size();
    isize_t getDroppedCount() const;

    /// \return whether a time index corresponds to a dropped data point
    /// \param inIndex the time sample index ranging from [0- (getNumTimes()-1)]
    bool isDropped(isize_t inIndex) const;

    /// \return The cropped frame ranges.
    ///
    const IndexRanges_t & getCropped() const;

    /// \return The number of cropped frames.
    ///
    isize_t getCroppedCount() const;

    /// \return         True if a time index corresponds to a cropped data point.
    /// \param inIndex  The time sample index to check.
    bool isCropped(isize_t inIndex) const;

    /// \return         True if the frame/time index is valid (e.g. not dropped or cropped), false otherwise.
    /// \param  inIndex The index of the frame/time to check.
    bool isIndexValid(const isize_t inIndex) const;

    /// \return The number of valid times (i.e. for which isIndexValid is true).
    ///
    isize_t getNumValidTimes() const;

    /// Converts a time index in the range [0- (getNumTimes()-1)] to the corresponding
    /// index in the file, accounting for dropped frames in the nVista system
    ///
    /// If the index correponds to a dropped or cropped index then the returned
    /// index will correspond to the last valid stored index before that or 0 if there
    /// no valid stored index. I.e. this function should not be used for dropped and
    /// cropped indices - if it is used as such, an assertion will fail in debug mode.
    ///
    /// \return the corresponding index as recorded in the file 
    /// \param inIndex the time index
    isize_t timeIdxToRecordedIdx(isize_t inIndex) const;

    /// \return a TimingInfo object initialized with default values
    /// \param inFrames total number of frames
    /// \param inDroppedFrames a vector of dropped frame numbers
    static TimingInfo getDefault(isize_t inFrames, const std::vector<isize_t> & inDroppedFrames);

private:

    /// The start time of the samples.
    Time m_start;

    /// The duration of one sample in seconds.
    DurationInSeconds m_step;

    /// The number of time samples.
    isize_t m_numTimes = 0;

    /// Whether the object is valid or not
    bool m_isValid = false;

    /// A vector of frame numbers indicating dropped frames
    std::vector<isize_t> m_droppedFrames;

    /// The cropped index ranges.
    IndexRanges_t m_cropped;

    /// Sets dropped frames, but also crops them (according to the current cropped
    /// frames) and sorts them.
    /// \param  inDroppedFrames     The desired dropped frames.
    void cropSortAndSetDroppedFrames(const std::vector<isize_t> & inDroppedFrames);

}; // class

/// type of container for multiple TimingInfo objects for MovieSeries, TraceSeries
using TimingInfos_t = std::vector<TimingInfo>;

/// Convert a global index to a pair of sample of segment indices.
///
/// This is used by the player to convert global frame indices to local pairs
/// of indices to read the right frame from the right file.
///
/// If the global index is in between segments, this returns <n, T_n> where n is
/// the segment before the index and T_n is the number of samples in that segment.
/// If the global index comes after the end of the last segment, then this
/// return <N, 0> where N is the number of segments.
///
/// \param  inGlobalTimingInfo  The timing info from which the global index lives in.
/// \param  inTimingInfos       The local timing info of each segment.
/// \param  inGlobalSampleIndex The global index to convert.
/// \return                     A pair containing the segment index, then the sample index.
std::pair<isize_t, isize_t>
getSegmentIndexAndSampleIndexFromGlobalSampleIndex(const TimingInfo inGlobalTimingInfo, const TimingInfos_t & inTimingInfos, isize_t inGlobalSampleIndex);

} // namespace isx

#endif // ISX_TIMING_INFO_H
