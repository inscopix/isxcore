#ifndef ISX_TIMINGINFO_H
#define ISX_TIMINGINFO_H

#include <cstdint>
#include "isxTime.h"

namespace isx
{

/// The timing info associated with temporal samples.
///
class TimingInfo
{
public:

    /// Default constructor.
    ///
    TimingInfo();

    /// Fully specified constructor.
    ///
    /// \param start    The start time of the samples.
    /// \param step     The duration of one sample.
    /// \param numTimes The number of samples.
    TimingInfo(isx::Time start, isx::Ratio step, uint32_t numTimes);

    /// Check validity of this.
    ///
    /// \return         True if this is valid.
    bool isValid() const;

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
