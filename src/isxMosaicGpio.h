#ifndef ISX_MOSAIC_GPIO_H
#define ISX_MOSAIC_GPIO_H

#include "isxGpio.h"

#include <memory>

namespace isx
{

class GpioFile;
template <typename T> class IoTaskTracker;

/// Class used to read and handle GPIO data
class MosaicGpio
    : public Gpio
    , public std::enable_shared_from_this<MosaicGpio>
{

public:

    /// Constructor
    ///
    MosaicGpio();

    /// Constructor
    /// \param inFileName the name of the GPIO data file
    MosaicGpio(const std::string & inFileName);

    /// Destructor
    ///
    ~MosaicGpio();

    // Overrides. See isxGpio.h for documentation.
    bool
    isValid() const override;

    bool
    isAnalog() const override;

    std::string
    getFileName() const override;

    isize_t
    numberOfChannels() override;

    const std::vector<std::string>
    getChannelList() const override;

    SpDTrace_t
    getAnalogData() override;

    void
    getAnalogDataAsync(GpioGetAnalogDataCB_t inCallback) override;

    SpLogicalTrace_t
    getLogicalData(const std::string & inChannelName) override;

    void
    getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback) override;

    const isx::TimingInfo &
    getTimingInfo() const override;

    isx::TimingInfos_t
    getTimingInfosForSeries() const override;

    void
    cancelPendingReads() override;

private:

    std::shared_ptr<GpioFile>      m_file;
    std::shared_ptr<IoTaskTracker<DTrace_t>>          m_analogIoTaskTracker;
    std::shared_ptr<IoTaskTracker<LogicalTrace>>      m_logicalIoTaskTracker;

}; // class MosaicGpio

} // namespace isx

#endif /// ISX_MOSAIC_GPIO_H
