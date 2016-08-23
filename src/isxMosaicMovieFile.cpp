#include "isxMosaicMovieFile.h"
#include "isxException.h"
#include "isxJsonUtils.h"

#include <fstream>

namespace isx
{

MosaicMovieFile::MosaicMovieFile()
    : m_valid(false)
{
}

MosaicMovieFile::MosaicMovieFile(const std::string & inFileName)
    : m_valid(false)
{
    initialize(inFileName);
}

MosaicMovieFile::MosaicMovieFile(
    const std::string & inFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo,
    DataType inDataType)
    : m_valid(false)
{
    initialize(inFileName, inTimingInfo, inSpacingInfo, inDataType);
}

MosaicMovieFile::~MosaicMovieFile()
{
}

void
MosaicMovieFile::initialize(const std::string & inFileName)
{
    m_fileName = inFileName;
    readHeader();
    // TODO sweet : check that data if of expected size.
    m_valid = true;
}

void
MosaicMovieFile::initialize(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType)
{
    m_fileName = inFileName;
    m_timingInfo = inTimingInfo;
    m_spacingInfo = inSpacingInfo;
    m_dataType = inDataType;
    writeHeader();
    writeZeroData();
    m_valid = true;
}

bool
MosaicMovieFile::isValid() const
{
    return m_valid;
}

void
MosaicMovieFile::readFrame(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame)
{
    std::ifstream file(m_fileName, std::ios::binary);
    seekForReadFrame(file, inFrameNumber);

    outFrame = std::make_shared<U16VideoFrame_t>(
        m_spacingInfo,
        sizeof(uint16_t) * m_spacingInfo.getNumColumns(),
        1, // numChannels
        m_timingInfo.convertIndexToTime(inFrameNumber),
        inFrameNumber);

    // Need to dump data first into a type dependent array then convert it
    isize_t frameSizeInBytes = getFrameSizeInBytes();
    switch (m_dataType)
    {
        case DataType::U16:
        {
            // No conversion needed
            file.read(reinterpret_cast<char*>(outFrame->getPixels()), frameSizeInBytes);
            break;
        }
        case DataType::F32:
        {
            // TODO sweet : we'll probably get lots of warning messages (one
            // for each frame), so this probably shouldn't stay
            ISX_LOG_WARNING("Reading float as uint16.");

            // Dump data into floating point array then convert to uint16
            isize_t numPixels = m_spacingInfo.getTotalNumPixels();
            std::vector<float> frameBuf(numPixels, 0);
            file.read(reinterpret_cast<char*>(frameBuf.data()), frameSizeInBytes);

            uint16_t * frameArray = outFrame->getPixels();
            for (isize_t i = 0; i < numPixels; ++i)
            {
                frameArray[i] = uint16_t(frameBuf[i]);
            }
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", m_dataType);
            break;
        }
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }
}

void
MosaicMovieFile::readFrame(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame)
{
    std::ifstream file(m_fileName, std::ios::binary);
    seekForReadFrame(file, inFrameNumber);

    outFrame = std::make_shared<F32VideoFrame_t>(
        m_spacingInfo,
        sizeof(float) * m_spacingInfo.getNumColumns(),
        1, // numChannels
        m_timingInfo.convertIndexToTime(inFrameNumber),
        inFrameNumber);

    // Need to dump data first into a type dependent array then convert it
    isize_t frameSizeInBytes = getFrameSizeInBytes();
    switch (m_dataType)
    {
        case DataType::U16:
        {
            // Dump data into uint16 array then convert to float
            isize_t numPixels = m_spacingInfo.getTotalNumPixels();
            std::vector<uint16_t> frameBuf(numPixels, 0);
            file.read(reinterpret_cast<char*>(frameBuf.data()), frameSizeInBytes);

            float * frameArray = outFrame->getPixels();
            for (isize_t i = 0; i < numPixels; ++i)
            {
                frameArray[i] = float(frameBuf[i]);
            }
            break;
        }
        case DataType::F32:
        {
            // No conversion needed
            file.read(reinterpret_cast<char*>(outFrame->getPixels()), frameSizeInBytes);
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", m_dataType);
            break;
        }
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }
}

void
MosaicMovieFile::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::in);
    seekForWriteFrame(file, inVideoFrame->getFrameIndex());

    isize_t frameSizeInBytes = getFrameSizeInBytes();
    switch (m_dataType)
    {
        case DataType::U16:
        {
            // No conversion needed
            file.write(reinterpret_cast<char *>(inVideoFrame->getPixels()), frameSizeInBytes);
            break;
        }
        case DataType::F32:
        {
            // Cast pixel values into float buffer then write
            std::vector<float> frameBuf(frameSizeInBytes);
            uint16_t * inFrameArray = inVideoFrame->getPixels();
            isize_t numPixels = m_spacingInfo.getTotalNumPixels();
            for (isize_t i = 0; i < numPixels; ++i)
            {
                frameBuf[i] = float(inFrameArray[i]);
            }
            file.write(reinterpret_cast<char *>(frameBuf.data()), frameSizeInBytes);
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", m_dataType);
            break;
        }
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error writing movie frame.", m_fileName);
    }
}

void
MosaicMovieFile::writeFrame(const SpF32VideoFrame_t & inVideoFrame)
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::in);
    seekForWriteFrame(file, inVideoFrame->getFrameIndex());

    isize_t frameSizeInBytes = getFrameSizeInBytes();
    switch (m_dataType)
    {
        case DataType::U16:
        {
            // TODO sweet : we'll probably get lots of warning messages (one
            // for each frame), so this probably shouldn't stay
            ISX_LOG_WARNING("Writing float as uint16.");

            // Cast pixel values into uint16 buffer then write
            std::vector<uint16_t> frameBuf(frameSizeInBytes);
            float * inFrameArray = inVideoFrame->getPixels();
            isize_t numPixels = m_spacingInfo.getTotalNumPixels();
            for (isize_t i = 0; i < numPixels; ++i)
            {
                frameBuf[i] = uint16_t(inFrameArray[i]);
            }
            file.write(reinterpret_cast<char *>(frameBuf.data()), frameSizeInBytes);
            break;
        }
        case DataType::F32:
        {
            // No conversion needed
            file.write(reinterpret_cast<char *>(inVideoFrame->getPixels()), frameSizeInBytes);
            break;
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", m_dataType);
            break;
        }
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error writing movie frame.", m_fileName);
    }
}

std::string
MosaicMovieFile::getFileName() const
{
    return m_fileName;
}

const isx::TimingInfo &
MosaicMovieFile::getTimingInfo() const
{
    return m_timingInfo;
}

const isx::SpacingInfo &
MosaicMovieFile::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
MosaicMovieFile::getDataType() const
{
    return m_dataType;
}

void
MosaicMovieFile::readHeader()
{
    std::ifstream file(m_fileName, std::ios::binary);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error while reading movie header: ", m_fileName);
    }

    std::string jsonStr;
    file.seekg(std::ios_base::beg);
    std::getline(file, jsonStr, '\0');
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error while reading movie header: ", m_fileName);
    }

    json j;
    try
    {
        j = json::parse(jsonStr);
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error while parsing movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing movie header.");
    }

    m_headerOffset = file.tellg();

    try
    {
        // TODO sweet : extra check to see if data type is recognized
        m_dataType = DataType(isize_t(j["dataType"]));
        std::string type = j["type"];
        if (type.compare("Movie") != 0)
        {
            ISX_THROW(isx::ExceptionDataIO, "Expected type to be Movie. Instead got ", type, ".");
        }
        m_timingInfo = convertJsonToTimingInfo(j["timingInfo"]);
        m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing movie header.");
    }
}

void
MosaicMovieFile::writeHeader()
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::trunc);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie when writing header: ", m_fileName);
    }

    json j;
    try
    {
        j["type"] = "Movie";
        j["dataType"] = isize_t(m_dataType);
        j["timingInfo"] = convertTimingInfoToJson(m_timingInfo);
        j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
        // TODO sweet : these aren't in the state of a movie right now, but they
        // are in the spec, so assigning to null for now
        j["dataRange"] = nullptr;
        j["displayRange"] = nullptr;
        j["mosaicVersion"] = CoreVersionVector();
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Error generating movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Unknown error while generating movie header.");
    }

    file << std::setw(4) << j;
    file << '\0';
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to write header in movie file: ", m_fileName);
    }

    m_headerOffset = file.tellp();
}

void
MosaicMovieFile::writeZeroData()
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::in);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie when writing zero data: ", m_fileName);
    }

    file.seekp(m_headerOffset);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to seek in movie file when writing zero data: ", m_fileName);
    }

    // Create a zero frame buffer once.
    isize_t frameSizeInBytes = getFrameSizeInBytes();
    std::vector<char> frameBuf(frameSizeInBytes, 0);

    // Write the frames to file one by one.
    isize_t numFrames = m_timingInfo.getNumTimes();
    for (isize_t i = 0; i < numFrames; ++i)
    {
        file.write(frameBuf.data(), frameSizeInBytes);
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to write zero data in movie file: ", m_fileName);
    }
}

isize_t
MosaicMovieFile::getPixelSizeInBytes() const
{
    switch (m_dataType)
    {
        case DataType::U16:
        {
            return sizeof(uint16_t);
        }
        case DataType::F32:
        {
            return sizeof(float);
        }
        default:
        {
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", m_dataType);
        }
    }
}

isize_t
MosaicMovieFile::getFrameSizeInBytes() const
{
    return (getPixelSizeInBytes() * m_spacingInfo.getTotalNumPixels());
}

void
MosaicMovieFile::seekForReadFrame(
        std::ifstream & inFile,
        isize_t inFrameNumber)
{
    if (!inFile.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file when reading frame: ", m_fileName);
    }

    // TODO sweet : check to see if time is outside of sample window instead
    // of reading last frame data
    isize_t numFrames = m_timingInfo.getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        inFrameNumber = numFrames - 1;
    }

    isize_t frameSizeInBytes = getFrameSizeInBytes();
    isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    inFile.seekg(m_headerOffset);
    inFile.seekg(offsetInBytes, std::ios_base::cur);
    if (!inFile.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for read.", m_fileName);
    }
}

void
MosaicMovieFile::seekForWriteFrame(
        std::ofstream & inFile,
        isize_t inFrameNumber)
{
    if (!inFile.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file when writing frame: ", m_fileName);
    }

    // TODO sweet : check to see if time is outside of sample window instead
    // of overwriting last frame data
    isize_t numFrames = m_timingInfo.getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        inFrameNumber = numFrames - 1;
    }

    isize_t frameSizeInBytes = getFrameSizeInBytes();
    isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    inFile.seekp(m_headerOffset);
    inFile.seekp(offsetInBytes, std::ios_base::cur);
    if (!inFile.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for write.", m_fileName);
    }
}

} // namespace isx
