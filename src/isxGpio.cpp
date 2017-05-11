#include "isxGpio.h"
#include "isxMosaicGpio.h"
#include "isxGpioSeries.h"

namespace isx
{

SpGpio_t
readGpio(const std::string & inFileName)
{
    return std::make_shared<MosaicGpio>(inFileName);
}

SpGpio_t
readGpioSeries(const std::vector<std::string> & inFileNames)
{
    return std::make_shared<GpioSeries>(inFileNames);
}

} // namespace isx
