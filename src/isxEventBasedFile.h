#ifndef ISX_EVENT_BASED_FILE_H
#define ISX_EVENT_BASED_FILE_H

#include <string>
#include <vector>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxTrace.h"

namespace isx
{

enum class SignalType : uint8_t
{
    SPARSE = 0,     ///< Signals containing event-triggered data.
    DENSE           ///< Signals containing data that's been sampled at a fixed sampling rate.
};

class EventBasedFile
{
public:
    /// \return True if this in a valid state, otherwise don't use it!
    ///
    virtual
    bool
    isValid() const = 0;

    /// \return The path of the file that stores the event data.
    virtual
    const std::string &
    getFileName() const = 0;

    /// \return The names of the channels stored here.
    virtual
    const std::vector<std::string>
    getChannelList() const = 0;

    /// \return The logical/sparse trace associated with the given channel name,
    ///         or nullptr if there is no such channel or this file is
    ///         still being written.
    ///         Note that if the channel is dense, this will successfully return
    ///         a sparse trace.
    virtual
    SpLogicalTrace_t
    getLogicalData(const std::string & inChannelName) = 0;

    /// \return The analog/dense trace associated with the given channel name,
    ///         or nullptr if there is no such dense channel or this file is
    ///         still being written.
    virtual
    SpFTrace_t
    getAnalogData(const std::string & inChannelName) = 0;

    /// \return The regular timing info associated with this file.
    ///         I think you should only trust the start and end time associated
    ///         with this. The step time and number of samples may be meaningful
    ///         in some cases (e.g. events detected from cell activity).
    virtual
    const isx::TimingInfo
    getTimingInfo() const = 0;
};

} // namespace isx

#endif /// ISX_EVENT_BASED_FILE_H
