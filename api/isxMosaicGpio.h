#ifndef ISX_GPIO_H
#define ISX_GPIO_H

#include "isxTimingInfo.h"
#include "isxCoreFwd.h"
#include "isxTrace.h"
#include "isxLogicalTrace.h"
#include "isxAsyncTaskResult.h"

#include <memory>
#include <string>
#include <functional>

namespace isx
{

class GpioFile;
template <typename T> class IoTaskTracker;

/// Class used to read and handle GPIO data
class MosaicGpio 
    : public std::enable_shared_from_this<MosaicGpio>
{

public:

    /// The type of callback for reading a trace from disk
    using GetAnalogDataCB_t = std::function<SpDTrace_t()>;
    /// The type of callback for getting an analog GPIO trace asynchronously
    using GpioGetAnalogDataCB_t = std::function<void(AsyncTaskResult<SpDTrace_t>)>;
    /// The type of callback for reading a logical trace from disk
    using GetLogicalDataCB_t = std::function<SpLogicalTrace_t()>;
    /// The type of callback for getting a logical trace asynchronously
    using GpioGetLogicalDataCB_t = std::function<void(AsyncTaskResult<SpLogicalTrace_t>)>;

    /// Constructor
    ///
    MosaicGpio();

    /// Constructor
    /// \param inFileName the name of the GPIO data file
    MosaicGpio(const std::string & inFileName);

    /// Destructor
    ///
    ~MosaicGpio();

    /// \return True if the gpio file is valid, false otherwise.
    ///
    bool 
    isValid() const; 

    /// \return true if this file contains data for an analog channel, false otherwise
    bool 
    isAnalog() const;

    /// \return     The name of the file.
    ///
    const std::string & 
    getFileName() const;

    /// \return the number of channels contained in the file
    ///
    const isize_t 
    numberOfChannels();

    /// \return a list of the channels contained in this file
    /// 
    const std::vector<std::string> 
    getChannelList() const;

    /// \return the trace for the analog channel or nullptr if the file doesn't contain analog data
    /// 
    SpDTrace_t 
    getAnalogData();

    /// Get an analog trace asynchronously
    /// \param inCallback 
    void 
    getAnalogDataAsync(GpioGetAnalogDataCB_t inCallback);

    /// \return the logical trace for the requested channel or nullptr if the file doesn't contain data for that channel
    /// \param inChannelName the name of the requested channel (as returned by getChannelList())
    SpLogicalTrace_t 
    getLogicalData(const std::string & inChannelName);

    /// Get an logical trace asynchronously
    /// \param inChannelName the name of the requested channel (as returned by getChannelList()) 
    /// \param inCallback 
    void 
    getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback);

    /// \return     The timing information read from the GPIO set.
    /// IMPORTANT: When interested in the timing info including dropped samples, make sure
    /// to get the timing info of the trace instead of this one. Only when the trace is read from
    /// the file, the list of dropped samples is constructed. 
    const isx::TimingInfo & 
    getTimingInfo() const;

private:

    std::shared_ptr<GpioFile>      m_file;
    std::shared_ptr<IoTaskTracker<DTrace_t>>          m_analogIoTaskTracker;
    std::shared_ptr<IoTaskTracker<LogicalTrace>>      m_logicalIoTaskTracker;

};

}

#endif /// ISX_GPIO_H