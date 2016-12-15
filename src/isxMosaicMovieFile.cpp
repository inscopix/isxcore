#include "isxMosaicMovieFile.h"
#include "isxException.h"
#include "isxJsonUtils.h"

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
    DataType inDataType)
    : m_valid(false)
{
    initialize(inFileName, inTimingInfo, inSpacingInfo, inDataType);
}

MosaicMovieFile::~MosaicMovieFile()
{
    if(m_file.is_open())
    {
        m_file.close();
        if (!m_file.good())
        {
            ISX_LOG_ERROR("Error closing the write file stream for file", m_fileName, 
            " eof: ", m_file.eof(), 
            " bad: ", m_file.bad(), 
            " fail: ", m_file.fail());
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
    m_timingInfos = TimingInfos_t{inTimingInfo};
    m_spacingInfo = inSpacingInfo;
    m_dataType = inDataType;
    m_file.open(m_fileName, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file for read/write: ", m_fileName);
    }
    writeHeader();
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
    const TimingInfo & ti = getTimingInfo();
    // TODO sweet : check to see if frame number exceeds number of frames
    // instead of returning the last frame.
    SpVideoFrame_t outFrame = std::make_shared<VideoFrame>(
        m_spacingInfo,
        getRowSizeInBytes(),
        1,
        m_dataType,
        ti.convertIndexToStartTime(inFrameNumber),
        inFrameNumber);

    if(ti.isDropped(inFrameNumber))
    {
        /// TODO: salpert 12/7/2016 return a placeholder frame displaying "No Data" or something similar
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        return outFrame;
    }

    // The frame was not dropped, shift frame numbers and proceed to read
    isize_t newFrameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
    seekForReadFrame(newFrameNumber);

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

void
MosaicMovieFile::readHeader()
{
    json j = readJsonHeader(m_file);
    m_headerOffset = m_file.tellg();

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
        m_timingInfos = TimingInfos_t{convertJsonToTimingInfo(j["timingInfo"])};
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
        j["timingInfo"] = convertTimingInfoToJson(getTimingInfo());
        j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
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
    
    writeJsonHeader(j, m_file);
    m_headerOffset = m_file.tellp();
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

    const isize_t frameSizeInBytes = getFrameSizeInBytes();
    const isize_t offsetInBytes = inFrameNumber * frameSizeInBytes;
    m_file.seekg(m_headerOffset);
    m_file.seekg(offsetInBytes, std::ios_base::cur);
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Error seeking movie frame for read.", m_fileName);
    }
}

void MosaicMovieFile::flush()
{
    m_file.flush();

    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, "Error flushing the file stream.");
    }
}



} // namespace isx
