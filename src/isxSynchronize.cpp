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

const char *
ExportAlignedTimestampsParams::getFilenameSuffix()
{
    // not needed for this operation
    return "";
}

const char *
ExportAlignedTimestampsParams::getOpName()
{
    return "Exporting";
}

const char *
ExportAlignedTimestampsParams::getNameForOutput()
{
    return "Exported";
}

ExportAlignedTimestampsParams
ExportAlignedTimestampsParams::fromString(const std::string & inStr)
{
    const json j = json::parse(inStr);
    ExportAlignedTimestampsParams params;
    params.m_refFilename = j.at("Timing Reference Filename").get<std::string>();
    params.m_alignFilenames = j.at("Align Filenames").get<std::vector<std::string>>();
    params.m_outputFilename = j.at("Output CSV Filename").get<std::string>();
    return params;
}

std::string
ExportAlignedTimestampsParams::toString() const
{
    json j;
    j["Timing Reference Filename"] = m_refFilename;
    j["Align Filename"] = m_alignFilenames;
    j["Output CSV Filename"] = m_outputFilename;
    return j.dump(4);
}

std::vector<std::string>
ExportAlignedTimestampsParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths = m_alignFilenames;
    inputFilePaths.insert(inputFilePaths.begin(), m_refFilename);
    return inputFilePaths;
}

DataSet::Type
ExportAlignedTimestampsParams::getOutputDataSetType()
{
    // not needed for this operation
    return DataSet::Type::MOVIE;
}

std::vector<std::string>
ExportAlignedTimestampsParams::getOutputFilePaths() const
{
    std::vector<std::string> flattenedOutputFn = {m_outputFilename};
    return flattenedOutputFn;
}

/// Wrapper around the getRecordingUUID function in isxMetadata
std::string getRecordingUUID(const std::string inFilename, const DataSet::Type inDataType)
{
    if (inDataType == DataSet::Type::MOVIE || inDataType == DataSet::Type::NVISION_MOVIE)
    {
        const auto movie = isx::readMovie(inFilename);
        return isx::getRecordingUUID(movie);
    }
    else if (inDataType == DataSet::Type::GPIO)
    {
        const auto gpio = isx::readGpio(inFilename);
        return isx::getRecordingUUID(gpio);
    }
    else
    {
        ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - can only get recording UUID of gpio files, isxd movies, and isxb movies.");
    }

    return "";
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
    // Check input filenames
    auto alignFilenames = inAlignFilenames;
    {
        std::sort(alignFilenames.begin(), alignFilenames.end());
        alignFilenames.erase(std::unique(alignFilenames.begin(), alignFilenames.end() ), alignFilenames.end());
        if (alignFilenames.size() != inAlignFilenames.size())
        {
            ISX_LOG_WARNING("Duplicate input align files found, removing from list of input align files.");
        }

        int index = -1;
        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
            if (alignFilename == inRefFilename)
            {
                index = static_cast<int>(i);
            }
        }

        if (index >= 0)
        {
            ISX_LOG_WARNING("Cannot align file to itself, removing from list of input align files.");
            alignFilenames.erase(alignFilenames.begin() + index);
        }
    }

    DataSet::Type refDataType;
    std::vector<DataSet::Type> alignDataTypes(alignFilenames.size());

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
        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
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

    // Check input files share the same recording UUID (and are therefore paired and synchronized)
    {
        const auto refRecordingUUID = getRecordingUUID(inRefFilename, refDataType);
        if (refRecordingUUID.empty())
        {
            ISX_THROW(isx::ExceptionUserInput, "Cannot determine if files are paired and synchronized - no recording UUID in timing reference file metadata.");
        }

        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
            const auto alignRecordingUUID = getRecordingUUID(alignFilename, alignDataTypes[i]);

            if (alignRecordingUUID != refRecordingUUID)
            {
                ISX_THROW(isx::ExceptionUserInput, "Files are not paired and synchronized - recording UUID of align file (", alignRecordingUUID, ") does not match recording UUID of timing reference file (", refRecordingUUID, ").");
            }
        }
    }

    const auto refStart = getStart(inRefFilename, refDataType);
    const auto refStartTimestamp = uint64_t(refStart.getSecsSinceEpoch().getNum());
    const auto refFirstTsc = getFirstTsc(inRefFilename, refDataType);

    for (size_t i = 0; i < alignFilenames.size(); i++)
    {
        const auto & alignFilename = alignFilenames[i];
        const auto alignStart = getStart(alignFilename, alignDataTypes[i]);
        const auto alignStartTimestamp = uint64_t(alignStart.getSecsSinceEpoch().getNum());
        const auto alignFirstTsc = getFirstTsc(alignFilename, alignDataTypes[i]);

        // Calculate the expected start timestamp of the align file based on the reference file
        // Convert the tsc values to int64_t in case the align file starts earlier than the reference file (i.e. negative tsc delta)
        // Convert the tsc delta to double in order to convert the value from microseconds to milliseconds
        const auto tscDelta = std::round(double(int64_t(alignFirstTsc) - int64_t(refFirstTsc)) / 1e3);

        // Convert the tsc delta and the reference timestamp to int64_t in order to handle signed arithmetic
        // Finally convert the result to uint64_t so it can be stored in the align file
        const uint64_t expAlignStartTimestamp = uint64_t(int64_t(refStartTimestamp) + int64_t(tscDelta));
        ISX_LOG_DEBUG("Synchronize start time diff: ", int64_t(expAlignStartTimestamp) - int64_t(alignStartTimestamp));
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

// Reads frame timestamps from a file of a specified data type
// Currently only supported for isxd movies, isxb movies, and gpio files
void
readTimestamps(
    const std::string inFilename,
    const DataSet::Type inDataType,
    std::vector<uint64_t> & outTimestamps,
    std::vector<std::pair<std::string, uint64_t>> & outChannels,
    TimingInfo & outTimingInfo

)
{
    if (inDataType == DataSet::Type::MOVIE || inDataType == DataSet::Type::NVISION_MOVIE)
    {
        const auto movie = isx::readMovie(inFilename);
        if (!movie->hasFrameTimestamps())
        {
            ISX_THROW(isx::ExceptionUserInput, "No frame timestamps stored in movie file to export.");
        }
        outTimingInfo = movie->getTimingInfo();

        outTimestamps.resize(outTimingInfo.getNumTimes());
        for (size_t i = 0; i < outTimingInfo.getNumTimes(); i++)
        {
            outTimestamps[i] = movie->getFrameTimestamp(i);
        }
    }
    else if (inDataType == DataSet::Type::GPIO)
    {
        const auto firstTsc = getFirstTsc(inFilename, inDataType);
        const auto gpio = isx::readGpio(inFilename);
        outTimingInfo = gpio->getTimingInfo();
        const auto startTime = outTimingInfo.getStart();

        const auto channelNames = gpio->getChannelList();
        const auto numChannels = channelNames.size();

        std::vector<SpLogicalTrace_t> logicalTraces;
        std::vector<SpFTrace_t> continuousTraces;
        gpio->getAllTraces(continuousTraces, logicalTraces);
        size_t numValues = 0;
        for (size_t c = 0; c < numChannels; c++)
        {
            const auto & channelName = channelNames[c];
            const auto traceValues = logicalTraces[c]->getValues();
            numValues += traceValues.size();
            outChannels.push_back(std::make_pair(channelName, numValues));
            for (const auto & tv : traceValues)
            {
                outTimestamps.push_back(firstTsc + uint64_t((tv.first - startTime).getNum()));
            }
        }
    }
    else
    {
        ISX_THROW(isx::ExceptionUserInput, "Unsupported data type - can only read timestamps from gpio files, isxd movies, and isxb movies.");
    }
}

AsyncTaskStatus exportAlignedTimestamps(
    ExportAlignedTimestampsParams inParams,
    SpExportAlignedTimestampsOutputParams_t outParams,
    AsyncCheckInCB_t inCheckInCB
)
{
    // Check input filenames
    auto alignFilenames = inParams.m_alignFilenames;
    auto alignNames = inParams.m_alignNames;
    {
        if (alignFilenames.size() != alignNames.size())
        {
            ISX_THROW(isx::ExceptionUserInput, "Number of align file paths does not match number of align names.");
        }
        std::sort(alignFilenames.begin(), alignFilenames.end());
        alignFilenames.erase(std::unique(alignFilenames.begin(), alignFilenames.end() ), alignFilenames.end());

        std::sort(alignNames.begin(), alignNames.end());
        alignNames.erase(std::unique(alignNames.begin(), alignNames.end() ), alignNames.end());
        if (alignFilenames.size() != inParams.m_alignFilenames.size())
        {
            ISX_LOG_WARNING("Duplicate input align files found, removing from list of input align files.");
        }

        int index = -1;
        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
            if (alignFilename == inParams.m_refFilename)
            {
                ISX_ASSERT(alignNames[i] == inParams.m_refName);
                index = static_cast<int>(i);
            }
        }

        if (index >= 0)
        {
            ISX_LOG_WARNING("Cannot align file to itself, removing from list of input align files.");
            alignFilenames.erase(alignFilenames.begin() + index);
            alignNames.erase(alignNames.begin() + index);
        }
    }

    DataSet::Type refDataType;
    std::vector<DataSet::Type> alignDataTypes(alignFilenames.size());
    // Check input data types
    {
        const std::vector<DataSet::Type> supportedRefDataTypes = {DataSet::Type::GPIO, DataSet::Type::MOVIE, DataSet::Type::NVISION_MOVIE};
        bool isRefDataTypeSupported = false;
        isx::DataSet::Properties props;
        refDataType = isx::readDataSetType(inParams.m_refFilename, props);
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

        const std::vector<DataSet::Type> supportedAlignDataTypes = {DataSet::Type::GPIO, DataSet::Type::MOVIE, DataSet::Type::NVISION_MOVIE};
        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
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

    // Check input files share the same recording UUID (and are therefore paired and synchronized)
    {
        const auto refRecordingUUID = getRecordingUUID(inParams.m_refFilename, refDataType);
        if (refRecordingUUID.empty())
        {
            ISX_THROW(isx::ExceptionUserInput, "Cannot determine if files are paired and synchronized - no recording UUID in timing reference file metadata.");
        }

        for (size_t i = 0; i < alignFilenames.size(); i++)
        {
            const auto & alignFilename = alignFilenames[i];
            const auto alignRecordingUUID = getRecordingUUID(alignFilename, alignDataTypes[i]);

            if (alignRecordingUUID != refRecordingUUID)
            {
                ISX_THROW(isx::ExceptionUserInput, "Files are not paired and synchronized - recording UUID of align file (", alignRecordingUUID, ") does not match recording UUID of timing reference file (", refRecordingUUID, ").");
            }
        }
    }

    // construct output csv
    std::vector<std::string> inputFilenames = alignFilenames;
    inputFilenames.insert(inputFilenames.begin(), inParams.m_refFilename);

    std::vector<std::string> inputNames = alignNames;
    inputNames.insert(inputNames.begin(), inParams.m_refName);

    std::vector<DataSet::Type> inputDataTypes = alignDataTypes;
    inputDataTypes.insert(inputDataTypes.begin(), refDataType);

    const size_t numInputs = inputFilenames.size();

    std::vector<std::vector<uint64_t>> inputTimestamps(numInputs);
    std::vector<std::vector<std::pair<std::string, uint64_t>>> inputChannels(numInputs);
    std::vector<TimingInfo> inputTimingInfos(numInputs);
    size_t maxFrames = 0;
    for (size_t i = 0; i < numInputs; i++)
    {
        readTimestamps(inputFilenames[i], inputDataTypes[i], inputTimestamps[i], inputChannels[i], inputTimingInfos[i]);
        maxFrames = std::max(inputTimestamps[i].size(), maxFrames);
    }

    const auto refFirstTsc = getFirstTsc(inParams.m_refFilename, refDataType);
    const auto refEpochStartTimestamp = double(getStart(inParams.m_refFilename, refDataType).getSecsSinceEpoch().getNum()) / 1e3;

    std::ofstream csv(inParams.m_outputFilename);
    for (size_t i = 0; i < numInputs; i++)
    {
        if (i > 0)
        {
            csv << ",";
        }

        csv << inputNames[i] << " Timestamp (s)";
        
        if (inputChannels[i].size())
        {
            csv << "," << inputNames[i] << " Channel";
        }
    }
    csv << std::endl;
    
    const size_t floatDecimalPrecision = 6;
    for (size_t f = 0; f < maxFrames; f++)
    {
        for (size_t i = 0; i < numInputs; i++)
        {
            if (i > 0)
            {
                csv << ",";
            }

            if (f >= inputTimestamps[i].size())
            {
                continue;
            }

            // Output timestamp in specified format
            const auto tsc = inputTimestamps[i][f];
            if (inParams.m_format == WriteTimeRelativeTo::FIRST_DATA_ITEM)
            {
                const auto timestamp = double(tsc - refFirstTsc) / 1e6;
                csv << std::fixed << std::setprecision(floatDecimalPrecision) << timestamp;
            }
            else if (inParams.m_format == WriteTimeRelativeTo::UNIX_EPOCH)
            {
                const auto timestamp = refEpochStartTimestamp + (double(tsc - refFirstTsc) / 1e6);
                csv << std::fixed << std::setprecision(floatDecimalPrecision) << timestamp;
            }
            else
            {
                csv << tsc;
            }

            if (inputChannels[i].size())
            {
                // Find the channel that this timestamp belongs to
                size_t channelIndex = 0;
                while (channelIndex < (inputChannels[i].size() - 1) && f > (inputChannels[i][channelIndex].second - 1))
                {
                    channelIndex++;
                }
                ISX_ASSERT(channelIndex < inputChannels[i].size());
                csv << "," << inputChannels[i][channelIndex].first;
            }
        }
        csv << std::endl;
    }

    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
