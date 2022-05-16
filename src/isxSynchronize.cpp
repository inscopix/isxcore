#include "isxSynchronize.h"
#include "isxGpio.h"
#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxNVisionMovieFile.h"
#include "isxMosaicMovieFile.h"

#include <json.hpp>
using json = nlohmann::json;

namespace isx
{

uint64_t getGpioFirstTsc(const SpGpio_t & inGpio)
{
    json extraProps = json::parse(inGpio->getExtraProperties());
    if (extraProps.find("firstTsc") == extraProps.end())
    {
        ISX_THROW(isx::ExceptionUserInput, "GPIO first tsc not stored in file metadata.");
    }

    return extraProps.at("firstTsc").get<uint64_t>();
}

void setIsxbStartTime(const std::string inIsxbFilename, const Time & inStartTime)
{
    const auto header = isx::NVisionMovieFile::Header();
    const auto offset = sizeof(header.m_fileVersion) + sizeof(header.m_headerSize);

    const uint64_t startTimestamp = uint64_t(inStartTime.getSecsSinceEpoch().getNum());
    const int64_t utcOffset = int64_t(inStartTime.getUtcOffset());
    
    std::fstream file(inIsxbFilename, std::ios::binary | std::ios_base::in | std::ios_base::out);
    file.seekp(offset, std::ios_base::beg);
    file.write(reinterpret_cast<const char*>(&startTimestamp), sizeof(startTimestamp));
    file.write(reinterpret_cast<const char*>(&utcOffset), sizeof(utcOffset));
    file.close();
}

void setIsxdStartTime(const std::string inIsxdFilename, const Time & inStartTime)
{
    MosaicMovieFile file(inIsxdFilename, true);
    const auto originalTi = file.getTimingInfo();
    const TimingInfo modifiedTi(
        inStartTime,
        originalTi.getStep(),
        originalTi.getNumTimes(),
        originalTi.getDroppedFrames(),
        originalTi.getCropped(),
        originalTi.getBlankFrames()
    );
    file.closeForWriting(modifiedTi);
}

AsyncTaskStatus synchronizeStartTimes(
    const std::string inGpioFilename,
    const std::string inIsxdFilename,
    const std::string inIsxbFilename
)
{
    // Read inputs
    SpGpio_t gpio = isx::readGpio(inGpioFilename);
    SpMovie_t isxbMovie = isx::readMovie(inIsxbFilename);
    SpMovie_t isxdMovie = isx::readMovie(inIsxdFilename);

    // Get epoch start timestamp and first TSC of gpio file
    const auto gpioStartTime = gpio->getTimingInfo().getStart();
    const auto gpioStartTimestamp = uint64_t(gpioStartTime.getSecsSinceEpoch().getNum());
    const auto gpioFirstTsc = getGpioFirstTsc(gpio);

    // Get first TSC of the isxb file
    uint64_t isxbFirstTsc;
    if (isxbMovie->getTimingInfo().isIndexValid(0))
    {
        isxbFirstTsc = isxbMovie->getFrameTimestamp(0);
    }
    else
    {
        // Handle case where there is no tsc value for the first frame of the movie
        const auto & ti = isxbMovie->getTimingInfo();
        size_t firstValidIdx = 0;
        for (size_t i = 1; i < ti.getNumTimes(); i++)
        {
            if (ti.isIndexValid(i))
            {
                firstValidIdx = i;
                break;
            }
        }

        if (firstValidIdx == 0)
        {
            ISX_THROW(isx::Exception, "Failed to find index of first valid frame in isxb file.");
        }

        const auto firstValidTsc = isxbMovie->getFrameTimestamp(firstValidIdx);
        const auto stepSizeUs = ti.getStep().toDouble() * 1e6;
        isxbFirstTsc = static_cast<uint64_t>(std::round(double(firstValidTsc) - stepSizeUs * double(firstValidIdx)));
    }

    // Set epoch start timestamp of isxb file
    // TSC values are in microseconds, but epoch timestamp is stored in milliseconds, so the epoch timestamp needs to be rounded to the closest millisecond
    const uint64_t isxbStartTimestamp = gpioStartTimestamp + static_cast<uint64_t>(std::round(double(isxbFirstTsc - gpioFirstTsc) / 1e3));
    const Time isxbStartTime(
        DurationInSeconds::fromMilliseconds(isxbStartTimestamp),
        gpioStartTime.getUtcOffset()
    );
    setIsxbStartTime(inIsxbFilename, isxbStartTime);

    // Get first TSC of the isxd file
    uint64_t isxdFirstTsc;
    if (isxdMovie->getTimingInfo().isIndexValid(0))
    {
        isxdFirstTsc = isxdMovie->getFrameTimestamp(0);
    }
    else
    {
        // Handle case where there is no tsc value for the first frame of the movie
        const auto & ti = isxdMovie->getTimingInfo();
        size_t firstValidIdx = 0;
        for (size_t i = 1; i < ti.getNumTimes(); i++)
        {
            if (ti.isIndexValid(i))
            {
                firstValidIdx = i;
                break;
            }
        }

        if (firstValidIdx == 0)
        {
            ISX_THROW(isx::Exception, "Failed to find index of first valid frame in isxd file.");
        }

        const auto firstValidTsc = isxdMovie->getFrameTimestamp(firstValidIdx);
        const auto stepSizeUs = ti.getStep().toDouble() * 1e6;
        isxdFirstTsc = static_cast<uint64_t>(std::round(double(firstValidTsc) - stepSizeUs * double(firstValidIdx)));
    }

    // Set epoch start timestamp of isxd file
    // TSC values are in microseconds, but epoch timestamp is stored in milliseconds, so the epoch timestamp needs to be rounded to the closest millisecond
    const uint64_t isxdStartTimestamp = gpioStartTimestamp + static_cast<uint64_t>(std::round(double(isxdFirstTsc - gpioFirstTsc) / 1e3));
    const Time isxdStartTime(
        DurationInSeconds::fromMilliseconds(isxdStartTimestamp),
        gpioStartTime.getUtcOffset()
    );
    setIsxdStartTime(inIsxdFilename, isxdStartTime);

    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
