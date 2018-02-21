#include "isxMosaicMovieFile.h"
#include "isxException.h"
#include "isxStopWatch.h"

#include <sys/stat.h>

#include <cstring>



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
    DataType inDataType,
    const bool inWriteFrameTimeStamps)
    : m_valid(false)
{
    initialize(inFileName, inTimingInfo, inSpacingInfo, inDataType, inWriteFrameTimeStamps);
}

MosaicMovieFile::~MosaicMovieFile()
{
    if (isValid())
    {
        if (!m_fileClosedForWriting)
        {
            ISX_LOG_ERROR("MosaicMovieFile destroyed before calling closeForWriting.  File may be corrupt: ", m_fileName);
            closeForWriting();
        }

        if (m_file.is_open() && m_file.good())
        {
            m_file.close();
            if (!m_file.good())
            {
                ISX_LOG_ERROR("Error closing the stream for file", m_fileName,
                " eof: ", m_file.eof(), 
                " bad: ", m_file.bad(), 
                " fail: ", m_file.fail());
            }
        }
    }
}

void
MosaicMovieFile::initialize(const std::string & inFileName)
{
    m_fileName = inFileName;
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file for reading: ", m_fileName);
    }
    readHeader();
    m_fileClosedForWriting = true;
    // TODO sweet : check that data if of expected size.
    m_valid = true;
}

void
MosaicMovieFile::initialize(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType,
        const bool inWriteFrameTimeStamps)
{
    m_fileName = inFileName;
    m_timingInfos = TimingInfos_t{inTimingInfo};
    m_spacingInfo = inSpacingInfo;
    m_dataType = inDataType;
    m_hasFrameTimeStamps = inWriteFrameTimeStamps;
    m_file.open(m_fileName, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file for read/write: ", m_fileName);
    }

    m_valid = true;
}

void
MosaicMovieFile::closeForWriting(const TimingInfo & inTimingInfo)
{
    if (isValid())
    {
        try
        {
            if (!m_fileClosedForWriting)
            {
                if (inTimingInfo.isValid())
                {
                    setTimingInfo(inTimingInfo);
                }

                writeHeader();
                m_fileClosedForWriting = true;
            }
        } 
        catch(isx::Exception &)
        {
        }
        catch(std::exception & e)
        {
            ISX_LOG_ERROR("Exception closing file ", m_fileName, ": ", e.what());
        }
        catch(...)
        {
            ISX_LOG_ERROR("Unkown exception closing file ", m_fileName);
        }
    }
}

void 
MosaicMovieFile::setTimingInfo(const TimingInfo & inTimingInfo)
{
    m_timingInfos = {inTimingInfo};
}
    
bool
MosaicMovieFile::isValid() const
{
    return m_valid;
}


SpVideoFrame_t
MosaicMovieFile::readFrame(isize_t inFrameNumber)
{
    const TimingInfo & ti = getTimingInfo();

    if (ti.isCropped(inFrameNumber))
    {
        SpVideoFrame_t outFrame = makeVideoFrame(inFrameNumber);
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::CROPPED);
        return outFrame;
    }
    else if (ti.isDropped(inFrameNumber))
    {
        SpVideoFrame_t outFrame = makeVideoFrame(inFrameNumber);
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::DROPPED);
        return outFrame;
    }

    // The frame was not dropped, shift frame numbers and proceed to read
    isize_t newFrameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
    seekForReadFrame(newFrameNumber);

    SpVideoFrame_t outFrame;
    if (hasFrameTimeStamps())
    {
        uint64_t microsecondsSinceStart = 0;
        m_file.read(reinterpret_cast<char *>(&microsecondsSinceStart), sizeof(microsecondsSinceStart));
        const Time timeStamp(DurationInSeconds(microsecondsSinceStart, isize_t(1E6)));
        outFrame = makeVideoFrame(inFrameNumber, timeStamp);
    }
    else
    {
        outFrame = makeVideoFrame(inFrameNumber);
    }

    m_file.read(outFrame->getPixels(), getFrameSizeInBytes());

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error reading movie frame.");
    }

    return outFrame;
}

void
MosaicMovieFile::writeFrame(const SpVideoFrame_t & inVideoFrame)
{
    if (m_fileClosedForWriting)
    {
        ISX_THROW(isx::ExceptionFileIO,
                  "Writing frame after file was closed for writing.", m_fileName);
    }

    const DataType frameDataType = inVideoFrame->getDataType();
    if (frameDataType == m_dataType)
    {
        m_file.write(inVideoFrame->getPixels(), getFrameSizeInBytes());
    }
    else
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Frame pixel type (", int(frameDataType),
                ") does not match movie data type (", int(m_dataType), ").");
    }

    // Write the timestamp after the frame data to do slightly less seeking
    // when reading the frame data alone.
    if (m_hasFrameTimeStamps)
    {
        const DurationInSeconds secondsSinceStart = inVideoFrame->getTimeStamp() - getTimingInfo().getStart();
        const uint64_t timeStamp = uint64_t(secondsSinceStart.toDouble() * 1E6);
        m_file.write(reinterpret_cast<const char*>(&timeStamp), sizeof(timeStamp));
    }

    m_headerOffset = m_file.tellp();

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error writing movie frame.", m_fileName);
    }

    flush();
}

std::string
MosaicMovieFile::getFileName() const
{
    return m_fileName;
}

const isx::TimingInfo &
MosaicMovieFile::getTimingInfo() const
{
    return m_timingInfos[0];
}

const isx::TimingInfos_t &
MosaicMovieFile::getTimingInfosForSeries() const
{
    return m_timingInfos;
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

SpVideoFrame_t
MosaicMovieFile::makeVideoFrame(const isize_t inIndex) const
{
    return makeVideoFrame(inIndex, getTimingInfo().convertIndexToStartTime(inIndex));
}

SpVideoFrame_t
MosaicMovieFile::makeVideoFrame(const isize_t inIndex, const Time & inTimeStamp) const
{
    const SpacingInfo & spacingInfo = getSpacingInfo();
    const DataType dataType = getDataType();
    const isize_t pixelSizeInBytes = getDataTypeSizeInBytes(dataType);
    const isize_t rowSizeInBytes = pixelSizeInBytes * spacingInfo.getNumColumns();
    SpVideoFrame_t outFrame = std::make_shared<VideoFrame>(
            spacingInfo,
            rowSizeInBytes,
            1,
            dataType,
            inTimeStamp,
            inIndex);
    return outFrame;
}

void
MosaicMovieFile::readHeader()
{
    json j = readJsonHeaderAtEnd(m_file, m_headerOffset);

    try
    {
        // TODO sweet : extra check to see if data type is recognized
        m_dataType = DataType(isize_t(j["dataType"]));
        DataSet::Type type = DataSet::Type(size_t(j["type"]));
        if (!(type == DataSet::Type::MOVIE || type == DataSet::Type::IMAGE))
        {
            ISX_THROW(ExceptionDataIO, "Expected type to be Movie or Image. Instead got ", size_t(type), ".");
        }
        m_timingInfos = TimingInfos_t{convertJsonToTimingInfo(j["timingInfo"])};
        m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
        // Some old test files don't have the fileVersion key.
        // I think that's also true for alpha versions of nVista file format
        // (because it's using an old version of the movie writer).
        size_t version = 0;
        if (j.find("fileVersion") != j.end())
        {
            version = size_t(j["fileVersion"]);
        }
        m_hasFrameTimeStamps = version > 0 && bool(j["hasFrameTimeStamps"]);
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
        const TimingInfo & ti = getTimingInfo();
        if (ti.getNumTimes() > 1)
        {
            j["type"] = size_t(DataSet::Type::MOVIE);
        }
        else
        {
            j["type"] = size_t(DataSet::Type::IMAGE);
        }
        j["timingInfo"] = convertTimingInfoToJson(ti);
        j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
        j["producer"] = getProducerAsJson();
        j["fileVersion"] = s_version;
        j["hasFrameTimeStamps"] = m_hasFrameTimeStamps;
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
    // Seek to end of file before writing header.
    // Linux was complaining when writing frames to a movie,
    // then reading from it and only after reading closing 
    // the file (and thus writing the header). flush was
    // causing a segfault
    m_file.seekp(0, std::ios_base::end);
    m_headerOffset = m_file.tellp();
    writeJsonHeaderAtEnd(j, m_file);
    flush();
}

isize_t
MosaicMovieFile::getPixelSizeInBytes() const
{
    isize_t sizeInBytes = getDataTypeSizeInBytes(m_dataType);
    if (sizeInBytes == 0)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Unrecognized pixel size type: ", m_dataType);
    }
    return sizeInBytes;
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
MosaicMovieFile::seekForReadFrame(isize_t inFrameNumber)
{
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file when reading frame: ", m_fileName);
    }

    const isize_t numFrames = getTimingInfo().getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "The index of the frame (", inFrameNumber, ") is out of range (0-",
            numFrames-1, ").");
    }

    size_t frameSizeInBytes = getFrameSizeInBytes();
    if (hasFrameTimeStamps())
    {
        frameSizeInBytes += sizeof(uint64_t);
    }

    const std::ios::pos_type offsetInBytes = inFrameNumber * frameSizeInBytes;
    m_file.seekg(offsetInBytes);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for read.", m_fileName);
    }
    if (offsetInBytes >= m_headerOffset)
    {
        m_file.setstate(std::ios::badbit);
    }
}

void
MosaicMovieFile::flush()
{
    m_file.flush();

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error flushing the file stream.");
    }
}

bool
MosaicMovieFile::hasFrameTimeStamps() const
{
    return m_hasFrameTimeStamps;
}

} // namespace isx
