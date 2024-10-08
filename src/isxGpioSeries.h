#ifndef ISX_GPIO_SERIES_H
#define ISX_GPIO_SERIES_H

#include "isxGpio.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A temporal series of GPIO sets.
///
class GpioSeries
    : public Gpio
    , public std::enable_shared_from_this<GpioSeries>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid GPIO series and is for allocation purposes only.
    GpioSeries();

    /// Read constructor.
    ///
    /// This opens existing GPIO files and reads information from their headers.
    ///
    /// \param  inFileNames     The paths of the GPIO files to read.
    /// \throw  ExceptionFileIO If reading one of the files fails.
    /// \throw  ExceptionDataIO If parsing one of the files fails.
    GpioSeries(const std::vector<std::string> & inFileNames);

    /// Destructor.
    ///
    ~GpioSeries();

    // Overrides
    bool
    isValid() const override;

    bool
    isAnalog(const std::string & inChannelName) const override;

    std::string
    getFileName() const override;

    isize_t
    numberOfChannels() override;

    const std::vector<std::string>
    getChannelList() const override;

    void 
    getAllTraces(std::vector<SpFTrace_t> & outContinuousTraces, std::vector<SpLogicalTrace_t> & outLogicalTraces) override;

    SpFTrace_t
    getAnalogData(const std::string & inChannelName) override;

    void
    getAnalogDataAsync(const std::string & inChannelName, GpioGetAnalogDataCB_t inCallback) override;

    SpLogicalTrace_t
    getLogicalData(const std::string & inChannelName) override;

    void
    getLogicalDataAsync(const std::string & inChannelName, GpioGetLogicalDataCB_t inCallback) override;

    isx::TimingInfo 
    getTimingInfo(const std::string & inChannelName) const override;

    isx::TimingInfos_t
    getTimingInfosForSeries(const std::string & inChannelName) const override;

    isx::TimingInfo 
    getTimingInfo() const override;
    
    isx::TimingInfos_t
    getTimingInfosForSeries() const override;

    void
    cancelPendingReads() override;

private:

    /// True if the GPIO series is valid, false otherwise.
    bool m_valid = false;

    std::map<std::string, TimingInfo>          m_gaplessTimingInfo; ///< only really useful for global number of times
    TimingInfo                                 m_generalGaplessTimingInfo;
    std::vector<SpGpio_t>   m_gpios;

}; // class GpioSeries

} // namespace isx

#endif // ISX_GPIO_SERIES_H
