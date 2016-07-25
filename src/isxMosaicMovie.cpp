#include "isxMosaicMovie.h"
#include "isxVideoFrame.h"
#include "isxException.h"
#include "isxLog.h"
#include "isxJsonUtils.h"
#include <fstream>

namespace isx
{

MosaicMovie::MosaicMovie()
    : m_valid(false)
    , m_fileAccess(FileAccessType::NONE)
{
}

MosaicMovie::MosaicMovie(const std::string & inFileName)
    : m_valid(false)
    , m_fileName(inFileName)
    , m_fileAccess(FileAccessType::NONE)
{
    openForReadOnly();
    readHeader();
    // TODO sweet : check that data if of expected size.
    m_valid = true;
}

MosaicMovie::MosaicMovie(
    const std::string & inFileName,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
    : m_valid(false)
    , m_fileName(inFileName)
    , m_timingInfo(inTimingInfo)
    , m_spacingInfo(inSpacingInfo)
    , m_fileAccess(FileAccessType::NONE)
{
    openForReadWrite();
    writeHeader();
    writeZeroData();
    m_valid = true;
}

MosaicMovie::~MosaicMovie()
{
}

bool
MosaicMovie::isValid() const
{
    return m_valid;
}

SpU16VideoFrame_t
MosaicMovie::getFrame(isize_t inFrameNumber)
{
    if (!(m_fileAccess == FileAccessType::READ_ONLY
            || m_fileAccess == FileAccessType::READ_WRITE))
    {
        ISX_THROW(isx::ExceptionFileIO, "Movie file is not open for read access.");
    }

    isize_t rowSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getNumColumns();
    isize_t frameSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getTotalNumPixels();

    // TODO sweet : check to see if frame number exceeds number of frames
    // instead of returning the last frame.
    Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
    SpU16VideoFrame_t frame = std::make_shared<U16VideoFrame_t>(
        m_spacingInfo,
        rowSizeInBytes,
        1, // numChannels
        frameTime, inFrameNumber);

    isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    m_file.seekg(m_headerOffset);
    m_file.seekg(offsetInBytes, std::ios_base::cur);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error seeking movie frame for read.");
    }

    m_file.read(reinterpret_cast<char*>(frame->getPixels()), frameSizeInBytes);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }

    return frame;
}

SpU16VideoFrame_t
MosaicMovie::getFrame(const Time & inTime)
{
    isize_t frameNum = m_timingInfo.convertTimeToIndex(inTime);
    return getFrame(frameNum);
}

void
MosaicMovie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
}

void
MosaicMovie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
{
    isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
    getFrameAsync(frameNumber, inCallback);
}

void
MosaicMovie::writeFrame(const SpU16VideoFrame_t & inVideoFrame)
{
    if (m_fileAccess != FileAccessType::READ_WRITE)
    {
        ISX_THROW(isx::ExceptionFileIO, "Movie file is not open for write access.");
    }

    isize_t frameSizeInBytes = sizeof(uint16_t) * m_spacingInfo.getTotalNumPixels();

    Time frameTime = inVideoFrame->getTimeStamp();
    // TODO sweet : check to see if time is outside of sample window instead
    // of overwriting first or last frame data?
    isize_t frameNumber = m_timingInfo.convertTimeToIndex(frameTime);

    isize_t offsetInBytes = frameNumber * frameSizeInBytes;
    m_file.seekp(m_headerOffset);
    m_file.seekp(offsetInBytes, std::ios_base::cur);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error seeking movie frame for write.");
    }

    m_file.write(reinterpret_cast<char*>(inVideoFrame->getPixels()), frameSizeInBytes);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error writing movie frame.");
    }
}

const isx::TimingInfo &
MosaicMovie::getTimingInfo() const
{
    return m_timingInfo;
}

const isx::SpacingInfo &
MosaicMovie::getSpacingInfo() const
{
    return m_spacingInfo;
}

std::string
MosaicMovie::getName()
{
    return m_fileName;
}

void
MosaicMovie::serialize(std::ostream & strm) const
{
    strm << m_fileName;
}

void
MosaicMovie::openForReadOnly()
{
    if (!m_file.is_open())
    {
        m_file.open(m_fileName, std::ios_base::in | std::ios_base::binary);
    }
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error opening movie file.");
    }
    m_fileAccess = FileAccessType::READ_ONLY;
}

void
MosaicMovie::openForReadWrite()
{
    if (!m_file.is_open())
    {
        m_file.open(m_fileName, std::ios_base::out | std::ios_base::binary);
    }
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error opening movie file.");
    }
    m_fileAccess = FileAccessType::READ_WRITE;
}

void
MosaicMovie::readHeader()
{
    if (!(m_fileAccess == FileAccessType::READ_ONLY
            || m_fileAccess == FileAccessType::READ_WRITE))
    {
        ISX_THROW(isx::ExceptionFileIO, "Movie file is not open for read access.");
    }

    std::string jsonStr;
    m_file.seekg(std::ios_base::beg);
    std::getline(m_file, jsonStr, '\0');
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error while reading movie header.");
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

    m_headerOffset = m_file.tellg();

    try
    {
        std::string dataType = j["dataType"];
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
MosaicMovie::writeHeader()
{
    if (m_fileAccess != FileAccessType::READ_WRITE)
    {
        ISX_THROW(isx::ExceptionFileIO, "Movie file is not open for read/write access.");
    }

    m_file.seekp(std::ios_base::beg);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error seeking to beginning of movie file.");
    }

    json j;
    try
    {
        j["type"] = "Movie";
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
        ISX_THROW(isx::ExceptionDataIO, "Error generating movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while generating movie header.");
    }

    m_file << std::setw(4) << j;
    m_file << '\0';
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error writing movie header.");
    }

    m_headerOffset = m_file.tellp();
}

void
MosaicMovie::writeZeroData()
{
    if (m_fileAccess != FileAccessType::READ_WRITE)
    {
        ISX_THROW(isx::ExceptionFileIO, "Movie file is not open for write access.");
    }

    // Create a zero frame buffer once.
    isize_t numPixels = m_spacingInfo.getTotalNumPixels();
    isize_t frameSizeInBytes = sizeof(uint16_t) * numPixels;
    std::vector<uint16_t> frameVec(numPixels, 0);
    char* frameBuf = reinterpret_cast<char*>(frameVec.data());

    // Write the frames to file one by one.
    for (isize_t i = 0; i < m_timingInfo.getNumTimes(); ++i)
    {
        m_file.write(frameBuf, frameSizeInBytes);
    }

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error writing zero movie data.");
    }

    m_file.flush();
}

} // namespace isx
