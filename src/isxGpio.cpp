#include "isxGpio.h"
#include "isxMosaicGpio.h"
#include "isxGpioSeries.h"
#include "isxPathUtils.h"
#include "isxGpioImporter.h"

#include <algorithm>

namespace isx
{

std::string
Gpio::getExtraProperties() const
{
    return "null";
}

SpGpio_t
readGpio(const std::string & inFileName, const std::string & inOutputIsxdDir)
{
    std::string ext = getExtension(inFileName);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "isxd")
    {
        return std::make_shared<MosaicGpio>(inFileName);
    }
    else
    {
        // If input is not an isxd file, must first convert to one.

        #if ISX_OS_WIN32
        const std::string tmpOutputDir = getDirName(inFileName);
        const isx::GpioDataParams inputParams(tmpOutputDir, inFileName);
        #else
        const isx::GpioDataParams inputParams(inOutputIsxdDir, inFileName);
        #endif

        auto outputParams = std::make_shared<isx::GpioDataOutputParams>();
        runGpioDataImporter(inputParams, outputParams, [](float){return false;});
        if (outputParams->filenames.empty())
        {
            return nullptr;
        }
        return std::make_shared<MosaicGpio>(outputParams->filenames[0]);
    }
}

SpGpio_t
readGpioSeries(const std::vector<std::string> & inFileNames)
{
    return std::make_shared<GpioSeries>(inFileNames);
}

bool
isGpioFileExtension(const std::string & inFileName)
{
    auto e = isx::getExtension(inFileName);
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);
    return ((e == "raw") || (e == "hdf5") || (e == "gpio"));
}

} // namespace isx
