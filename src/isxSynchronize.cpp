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

void setStartTime(
    const std::string inFilename,
    const Time inStartTime,
    const DataSet::Type inDataType
)
{
    if (inDataType == DataSet::Type::MOVIE)
    {
        setIsxdStartTime(inFilename, inStartTime);
    }
    else if (inDataType == DataSet::Type::NVISION_MOVIE)
    {
        setIsxbStartTime(inFilename, inStartTime);
    }
    else
    {
        ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - can only set start time of isxd and isxb movies.");
    }
}

uint64_t getFirstTsc(
    const std::string inFilename,
    const DataSet::Type inDataType
)
{
    if (inDataType == DataSet::Type::MOVIE || inDataType == DataSet::Type::NVISION_MOVIE)
    {
        const auto movie = isx::readMovie(inFilename);
        if (movie->hasFrameTimestamps())
        {
            // Get first TSC of the movie
            if (movie->getTimingInfo().isIndexValid(0))
            {
                return movie->getFrameTimestamp(0);
            }
            else
            {
                // Handle case where there is no tsc value for the first frame of the movie
                const auto & ti = movie->getTimingInfo();
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

                const auto firstValidTsc = movie->getFrameTimestamp(firstValidIdx);
                const auto stepSizeUs = ti.getStep().toDouble() * 1e6;
                return static_cast<uint64_t>(std::round(double(firstValidTsc) - stepSizeUs * double(firstValidIdx)));
            }
        }
        else
        {
            ISX_THROW(isx::ExceptionUserInput, "Cannot get first tsc from movie with no frame timestamps.");
        }
    }
    else if (inDataType == DataSet::Type::GPIO)
    {
        const auto gpio = isx::readGpio(inFilename);
        json extraProps = json::parse(gpio->getExtraProperties());
        if (extraProps.find("firstTsc") == extraProps.end())
        {
            ISX_THROW(isx::ExceptionUserInput, "GPIO first tsc not stored in file metadata.");
        }

        return extraProps.at("firstTsc").get<uint64_t>();
    }
    else
    {
        ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - can only get first tsc of gpio files, isxd movies, and isxb movies.");
    }

    return 0;
}

Time getStart(
    const std::string inFilename,
    const DataSet::Type inDataType
)
{
    if (inDataType == DataSet::Type::MOVIE || inDataType == DataSet::Type::NVISION_MOVIE)
    {
        const auto movie = isx::readMovie(inFilename);
        return movie->getTimingInfo().getStart();
    }
    else if (inDataType == DataSet::Type::GPIO)
    {
        const auto gpio = isx::readGpio(inFilename);
        return gpio->getTimingInfo().getStart();
    }
    else
    {
        ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - can only get start time of gpio files, isxd movies, and isxb movies.");
    }

    return Time();
}

AsyncTaskStatus synchronizeStartTimes(
    const std::string inRefFilename,
    const std::vector<std::string> inAlignFilenames
)
{
    DataSet::Type refDataType;
    std::vector<DataSet::Type> alignDataTypes(inAlignFilenames.size());

    // Check input data types
    {
        const std::vector<DataSet::Type> supportedRefDataTypes = {DataSet::Type::GPIO, DataSet::Type::MOVIE, DataSet::Type::NVISION_MOVIE};
        
        bool isRefDataTypeSupported = false;
        isx::DataSet::Properties props;
        refDataType = isx::readDataSetType(inRefFilename, props);
        for (const auto & supportedRefDataType : supportedRefDataTypes)
        {
            if (refDataType == supportedRefDataType)
            {
                isRefDataTypeSupported = true;
                break;
            }
        }

        if (!isRefDataTypeSupported)
        {
            ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - only gpio files, isxd movies, and isxb movies are supported as a timing reference.");
        }

        const std::vector<DataSet::Type> supportedAlignDataTypes = {DataSet::Type::MOVIE, DataSet::Type::NVISION_MOVIE};

        for (size_t i = 0; i < inAlignFilenames.size(); i++)
        {
            const auto & alignFilename = inAlignFilenames[i];
            bool isAlignDataTypeSupported = false;
            isx::DataSet::Properties props;
            alignDataTypes[i] = isx::readDataSetType(alignFilename, props);
            for (const auto & supportedAlignDataType : supportedAlignDataTypes)
            {
                if (alignDataTypes[i] == supportedAlignDataType)
                {
                    isAlignDataTypeSupported = true;
                    break;
                }
            }

            if (!isAlignDataTypeSupported)
            {
                ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - only isxd movies and isxb movies are supported as input files to align to a timing reference.");
            }
        }
    }

    const auto refStart = getStart(inRefFilename, refDataType);
    const auto refStartTimestamp = uint64_t(refStart.getSecsSinceEpoch().getNum());
    const auto refFirstTsc = getFirstTsc(inRefFilename, refDataType);

    for (size_t i = 0; i < inAlignFilenames.size(); i++)
    {
        const auto & alignFilename = inAlignFilenames[i];
        const auto alignStart = getStart(alignFilename, alignDataTypes[i]);
        const auto alignStartTimestamp = uint64_t(alignStart.getSecsSinceEpoch().getNum());
        const auto alignFirstTsc = getFirstTsc(alignFilename, alignDataTypes[i]);

        const uint64_t expAlignStartTimestamp = refStartTimestamp + static_cast<uint64_t>(std::round(double(alignFirstTsc - refFirstTsc) / 1e3));
        if (alignStartTimestamp != expAlignStartTimestamp)
        {
            const Time newStart(
                DurationInSeconds::fromMilliseconds(expAlignStartTimestamp),
                refStart.getUtcOffset()
            );
            setStartTime(alignFilename, newStart, alignDataTypes[i]);
        }
    }
    
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
