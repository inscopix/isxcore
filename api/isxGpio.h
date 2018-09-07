#ifndef ISX_GPIO_H
#define ISX_GPIO_H

#include "isxTimingInfo.h"
#include "isxCoreFwd.h"
#include "isxTrace.h"
#include "isxLogicalTrace.h"
#include "isxAsyncTaskResult.h"

#include <string>
#include <functional>

namespace isx
{

/// Interface for GPIO data.
class Gpio
{

public:

    /// The type of callback for reading a trace from disk
    using GetAnalogDataCB_t = std::function<SpFTrace_t()>;
    /// The type of callback for getting an analog GPIO trace asynchronously
    using GpioGetAnalogDataCB_t = std::function<void(AsyncTaskResult<SpFTrace_t>)>;
    /// The type of callback for reading a logical trace from disk
    using GetLogicalDataCB_t = std::function<SpLogicalTrace_t()>;
    /// The type of callback for getting a logical trace asynchronously
    using GpioGetLogicalDataCB_t = std::function<void(AsyncTaskResult<SpLogicalTrace_t>)>;

    /// \return True if the gpio file is valid, false otherwise.
    ///
    virtual
    bool
    isValid() const = 0;

    /// \return true if the selected channel contains analog data, false otherwise
    /// \param inChannelName the name of the requested channel
    virtual
    bool
    isAnalog(const std::string & inChannelName) const = 0;

    /// \return     The name of the file.
    ///
    virtual
    std::string
    getFileName() const = 0;

    /// \return the number of channels contained in the file
    ///
    virtual
    isize_t
    numberOfChannels() = 0;

    /// \return a list of the channels contained in this file
    ///
    virtual
    const std::vector<std::string>
    getChannelList() const = 0;

    /// Get all the traces in the file
    /// Note that continous traces can also be read as logical traces, so any trace acquired as 
    /// dense data (continous sampling) will be returned in both formats. 
    /// \param outContinuousTraces the vector of traces. Sparse traces will correspond to null objects in this vector
    /// \param outLogicalTraces the vector of logical traces.
    virtual
    void 
    getAllTraces(std::vector<SpFTrace_t> & outContinuousTraces, std::vector<SpLogicalTrace_t> & outLogicalTraces) = 0;

    /// \return the trace for the analog channel or nullptr if the file doesn't contain analog data
    /// \param inChannelName the name of the requested channel
    virtual
    SpFTrace_t
    getAnalogData(const std::string & inChannelName) = 0;

    /// Get an analog trace asynchronously
    /// \param inChannelName
    /// \param inCallback
    virtual
    void
    getAnalogDataAsync(const std::string & inChannelName, GpioGetAnalogDataCB_t inCallback) = 0;

    /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    virtual
    SpLogicalTrace_t
    getLogicalData(const std::string & inChannelName) = 0;

    /// Get an logical trace asynchronously
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    /// \param inCallback
    virtual
    void
    getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback) = 0;

    /// \return     The timing information read from the GPIO set.
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    /// IMPORTANT: When interested in the timing info including dropped samples, make sure
    /// to get the timing info of the trace instead of this one. Only when the trace is read from
    /// the file, the list of dropped samples is constructed.
    virtual
    isx::TimingInfo 
    getTimingInfo(const std::string & inChannelName) const = 0;

    /// \return     The TimingInfos of a GpioSeries.
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    ///             For a regular GPIO set this will contain one TimingInfo object
    ///             matching getTimingInfo.
    virtual
    isx::TimingInfos_t
    getTimingInfosForSeries(const std::string & inChannelName) const = 0;

    /// \return     The general timing information read from the GPIO set.
    /// 
    virtual
    isx::TimingInfo 
    getTimingInfo() const = 0;

    /// \return     The general TimingInfos of a GpioSeries.
    /// 
    virtual
    isx::TimingInfos_t
    getTimingInfosForSeries() const = 0;

    /// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
    ///
    virtual
    void
    cancelPendingReads() = 0;

    /// \return     The extra properties of this movie which might include things
    ///             from nVista 3. The string is in JSON format.
    virtual
    std::string
    getExtraProperties() const;

}; // class Gpio

/// Read a GPIO set from a file.
/// \param  inFileName  The path of the GPIO file.
/// \return             The GPIO set read.
SpGpio_t
readGpio(const std::string & inFileName);

/// Attempt to read a series of GPIO sets from files.
/// \param  inFileNames The paths of the GPIO files.
/// \return             The series of GPIO sets.
SpGpio_t
readGpioSeries(const std::vector<std::string> & inFileNames);

} // namespace isx

#endif /// ISX_GPIO_H
