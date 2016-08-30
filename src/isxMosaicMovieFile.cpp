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

SpVideoFrame_t
MosaicMovieFile::readFrame(isize_t inFrameNumber)
{
    std::ifstream file(m_fileName, std::ios::binary);
    seekForReadFrame(file, inFrameNumber);

    // TODO sweet : check to see if frame number exceeds number of frames
    // instead of returning the last frame.
    SpVideoFrame_t outFrame = std::make_shared<VideoFrame>(
        m_spacingInfo,
        getRowSizeInBytes(),
        1,
        m_dataType,
        m_timingInfo.convertIndexToStartTime(inFrameNumber),
        inFrameNumber);

    file.read(outFrame->getPixels(), getFrameSizeInBytes());

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }

    return outFrame;
}

void
MosaicMovieFile::writeFrame(const SpVideoFrame_t & inVideoFrame)
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::in);
    seekForWriteFrame(file, inVideoFrame->getFrameIndex());

    const DataType frameDataType = inVideoFrame->getDataType();
    if (frameDataType == m_dataType)
    {
        file.write(inVideoFrame->getPixels(), getFrameSizeInBytes());
    }
    else
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Frame pixel type (", int(frameDataType),
                ") does not match movie data type (", int(m_dataType), ").");
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
    json j = readJsonHeader(file);
    m_headerOffset = file.tellg();

    try
    {
        // TODO sweet : extra check to see if data type is recognized
        m_dataType = DataType(isize_t(j["dataType"]));
        DataSet::Type type = DataSet::Type(size_t(j["type"]));
        if (type != DataSet::Type::MOVIE)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be Movie. Instead got ", size_t(type), ".");
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
    json j;
    try
    {
        j["dataType"] = isize_t(m_dataType);
        j["type"] = size_t(DataSet::Type::MOVIE);
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

    std::ofstream file(m_fileName, std::ios::binary | std::ios::trunc);
    writeJsonHeader(j, file);
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
    const isize_t frameSizeInBytes = getFrameSizeInBytes();
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
            ISX_THROW(isx::ExceptionDataIO, "Invalid pixel size type: ", int(m_dataType));
        }
    }
}

isize_t
MosaicMovieFile::getRowSizeInBytes() const
{
    return (getPixelSizeInBytes() * m_spacingInfo.getNumColumns());
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
    const isize_t numFrames = m_timingInfo.getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        inFrameNumber = numFrames - 1;
    }

    const isize_t frameSizeInBytes = getFrameSizeInBytes();
    const isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
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
    const isize_t numFrames = m_timingInfo.getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        inFrameNumber = numFrames - 1;
    }

    const isize_t frameSizeInBytes = getFrameSizeInBytes();
    const isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    inFile.seekp(m_headerOffset);
    inFile.seekp(offsetInBytes, std::ios_base::cur);
    if (!inFile.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for write.", m_fileName);
    }
}

} // namespace isx
