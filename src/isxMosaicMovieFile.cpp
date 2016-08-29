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
    const SpacingInfo & inSpacingInfo)
    : m_valid(false)
{
    initialize(inFileName, inTimingInfo, inSpacingInfo);
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
MosaicMovieFile::initialize(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo)
{
    m_fileName = inFileName;
    m_timingInfo = inTimingInfo;
    m_spacingInfo = inSpacingInfo;
    writeHeader();
    writeZeroData();
    m_valid = true;
}

bool
MosaicMovieFile::isValid() const
{
    return m_valid;
}

SpU16VideoFrame_t
MosaicMovieFile::readFrame(isize_t inFrameNumber)
{
    std::ifstream file(m_fileName, std::ios::binary);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file when reading frame: ", m_fileName);
    }

    isize_t rowSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getNumColumns();
    isize_t frameSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getTotalNumPixels();

    // TODO sweet : check to see if frame number exceeds number of frames
    // instead of returning the last frame.
    Time frameTime = m_timingInfo.convertIndexToStartTime(inFrameNumber);
    SpU16VideoFrame_t frame = std::make_shared<U16VideoFrame_t>(
        m_spacingInfo,
        rowSizeInBytes,
        1, // numChannels
        frameTime, inFrameNumber);

    isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    file.seekg(m_headerOffset);
    file.seekg(offsetInBytes, std::ios_base::cur);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error seeking movie frame for read.");
    }

    file.read(reinterpret_cast<char*>(frame->getPixels()), frameSizeInBytes);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }

    return frame;
}

void
MosaicMovieFile::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
    std::ofstream file(m_fileName, std::ios::binary | std::ios::in);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file when writing frame: ", m_fileName);
    }

    isize_t frameSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getTotalNumPixels();

    //// TODO sweet : check to see if time is outside of sample window instead
    //// of overwriting first or last frame data?
    isize_t frameNumber = inVideoFrame->getFrameIndex();
    isize_t numFrames = m_timingInfo.getNumTimes();
    if (frameNumber >= numFrames)
    {
        frameNumber = numFrames - 1;
    }

    isize_t offsetInBytes = frameNumber * frameSizeInBytes;
    file.seekp(m_headerOffset);
    file.seekp(offsetInBytes, std::ios_base::cur);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for write.", m_fileName);
    }

    file.write(reinterpret_cast<char*>(inVideoFrame->getPixels()), frameSizeInBytes);
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

void
MosaicMovieFile::readHeader()
{
    std::ifstream file(m_fileName, std::ios::binary);
    json j = readJsonHeader(file);
    m_headerOffset = file.tellg();

    try
    {
        std::string dataType = j["dataType"];
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
        j["type"] = size_t(DataSet::Type::MOVIE);
        // TODO sweet : data type is uint16 for now, but needs to be more general
        j["dataType"] = "uint16";
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
    isize_t numPixels = m_spacingInfo.getTotalNumPixels();
    isize_t frameSizeInBytes = sizeof(uint16_t) * numPixels;
    std::vector<uint16_t> frameVec(numPixels, 0);
    char* frameBuf = reinterpret_cast<char*>(frameVec.data());

    // Write the frames to file one by one.
    for (isize_t i = 0; i < m_timingInfo.getNumTimes(); ++i)
    {
        file.write(frameBuf, frameSizeInBytes);
    }

    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to write zero data in movie file: ", m_fileName);
    }
}

} // namespace isx
