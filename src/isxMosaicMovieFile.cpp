#include "isxMosaicMovieFile.h"
#include "isxException.h"
#include "isxStopWatch.h"
#include "isxCore.h"

#include <sys/stat.h>

#include <cstring>

#define ISX_DEBUG_FRAME_TIME 0
#if ISX_DEBUG_FRAME_TIME
#define ISX_LOG_DEBUG_FRAME_TIME(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_LOG_DEBUG_FRAME_TIME(...)
#endif

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
    const bool inHasFrameHeaderFooter)
    : m_valid(false)
{
    initialize(inFileName, inTimingInfo, inSpacingInfo, inDataType, inHasFrameHeaderFooter);
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

        isx::closeFileStreamWithChecks(m_file, m_fileName);
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
    m_valid = true;
    m_readOnly = true;
}

void
MosaicMovieFile::initialize(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType,
        const bool inHasFrameHeaderFooter)
{
    m_fileName = inFileName;
    m_timingInfos = TimingInfos_t{inTimingInfo};
    m_spacingInfo = inSpacingInfo;
    m_dataType = inDataType;
    m_hasFrameHeaderFooter = inHasFrameHeaderFooter;
    m_file.open(m_fileName, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open movie file for read/write: ", m_fileName);
    }

    m_valid = true;

    m_readOnly = false;
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

    SpVideoFrame_t outFrame = makeVideoFrame(inFrameNumber);

    if (ti.isCropped(inFrameNumber))
    {
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::CROPPED);
        return outFrame;
    }
    else if (ti.isDropped(inFrameNumber))
    {
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::DROPPED);
        return outFrame;
    }

    // The frame was not dropped, shift frame numbers and proceed to read
    seekForReadFrame(ti.timeIdxToRecordedIdx(inFrameNumber), true, false);

    m_file.read(outFrame->getPixels(), outFrame->getImageSizeInBytes());

    checkFileGood("Error reading movie frame");

    return outFrame;
}

std::vector<uint16_t>
MosaicMovieFile::readFrameHeader(const isize_t inFrameNumber)
{
    std::vector<uint16_t> header;
    const TimingInfo & ti = getTimingInfo();
    if (ti.isIndexValid(inFrameNumber))
    {
        seekForReadFrame(ti.timeIdxToRecordedIdx(inFrameNumber), false, false);
        header.resize(s_numHeaderFooterValues);
        m_file.read(reinterpret_cast<char *>(header.data()), s_headerFooterSizeInBytes);
        checkFileGood("Error reading movie frame header");
    }
    return header;
}

std::vector<uint16_t>
MosaicMovieFile::readFrameFooter(const isize_t inFrameNumber)
{
    std::vector<uint16_t> footer;
    const TimingInfo & ti = getTimingInfo();
    if (ti.isIndexValid(inFrameNumber))
    {
        seekForReadFrame(ti.timeIdxToRecordedIdx(inFrameNumber), true, true);
        footer.resize(s_numHeaderFooterValues);
        m_file.read(reinterpret_cast<char *>(footer.data()), s_headerFooterSizeInBytes);
        checkFileGood("Error reading movie frame footer");
    }
    return footer;
}

void
MosaicMovieFile::writeFrame(const SpVideoFrame_t & inVideoFrame)
{
    checkFileNotClosedForWriting();
    checkDataType(inVideoFrame->getDataType());

    m_file.write(inVideoFrame->getPixels(), getFrameSizeInBytes());
    m_headerOffset = m_file.tellp();

    checkFileGood("Error writing movie frame");
    flush();
}

void
MosaicMovieFile::writeFrameWithHeaderFooter(const uint16_t * inHeader, const uint16_t * inPixels, const uint16_t * inFooter)
{
    checkFileNotClosedForWriting();
    checkDataType(DataType::U16);

    m_file.write(reinterpret_cast<const char *>(inHeader), s_headerFooterSizeInBytes);
    m_file.write(reinterpret_cast<const char *>(inPixels), getFrameSizeInBytes());
    m_file.write(reinterpret_cast<const char *>(inFooter), s_headerFooterSizeInBytes);
    m_headerOffset = m_file.tellp();

    checkFileGood("Error writing movie frame");
    flush();
}

void
MosaicMovieFile::writeFrameWithHeaderFooter(const uint16_t * inBuffer)
{
    checkFileNotClosedForWriting();
    checkDataType(DataType::U16);

    m_file.write(reinterpret_cast<const char *>(inBuffer), (2 * s_headerFooterSizeInBytes) + getFrameSizeInBytes());
    m_headerOffset = m_file.tellp();

    checkFileGood("Error writing movie frame. " + m_fileName);
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
    return std::make_shared<VideoFrame>(
            getSpacingInfo(),
            getRowSizeInBytes(),
            1,
            getDataType(),
            getTimingInfo().convertIndexToStartTime(inIndex),
            inIndex);
}

void
MosaicMovieFile::setExtraProperties(const std::string & inProperties)
{
    try
    {
        m_extraProperties = json::parse(inProperties);
    }
    catch (const std::exception & error)
    {
        ISX_THROW(ExceptionDataIO, "Error parsing extra properties: ", error.what());
    }
}

std::string
MosaicMovieFile::getExtraProperties() const
{
    return m_extraProperties.dump();
}

SpacingInfo
MosaicMovieFile::getOriginalSpacingInfo() const
{
    if (m_extraProperties != nullptr)
    {
        return SpacingInfo::getDefaultForNVista3();
    }
    return SpacingInfo::getDefault();
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
        m_hasFrameHeaderFooter = version > 0 && bool(j["hasFrameHeaderFooter"]);
        if (j.find("extraProperties") != j.end())
        {
            m_extraProperties = j["extraProperties"];
        }
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
        j["hasFrameHeaderFooter"] = m_hasFrameHeaderFooter;
        j["extraProperties"] = m_extraProperties;
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
MosaicMovieFile::seekForReadFrame(isize_t inFrameNumber, const bool inSkipHeader, const bool inSkipFrame)
{
    checkFileGood("Movie file is bad before seeking for frame " + std::to_string(inFrameNumber));

    const isize_t numFrames = getTimingInfo().getNumTimes();
    if (inFrameNumber >= numFrames)
    {
        ISX_THROW(ExceptionDataIO, "The index of the frame (", inFrameNumber,
                ") is out of range (0-", numFrames - 1, ").");
    }

    const size_t frameSizeInBytes = getFrameSizeInBytes();
    size_t frameHeaderFooterSizeInBytes = frameSizeInBytes;
    if (m_hasFrameHeaderFooter)
    {
        frameHeaderFooterSizeInBytes += 2 * s_headerFooterSizeInBytes;
    }

    std::ios::pos_type offsetInBytes = inFrameNumber * frameHeaderFooterSizeInBytes;

    if (inSkipHeader && m_hasFrameHeaderFooter)
    {
        offsetInBytes += s_headerFooterSizeInBytes;
    }

    if (inSkipFrame)
    {
        offsetInBytes += frameSizeInBytes;
    }

    m_file.seekg(offsetInBytes);
    checkFileGood("Failed to seek in movie file when reading frame " + std::to_string(inFrameNumber));

    if (offsetInBytes >= m_headerOffset)
    {
        m_file.setstate(std::ios::badbit);
    }
}

void
MosaicMovieFile::flush()
{
    m_file.flush();
    checkFileGood("Error flushing the file stream");
}

void
MosaicMovieFile::checkFileNotClosedForWriting() const
{
    if (m_fileClosedForWriting)
    {
        ISX_THROW(ExceptionFileIO, "Writing frame after file was closed for writing: ", m_fileName);
    }
}

void
MosaicMovieFile::checkDataType(const DataType inDataType) const
{
    if (inDataType != m_dataType)
    {
        ISX_THROW(ExceptionDataIO, "Frame pixel type (", int(inDataType),
                ") does not match movie data type (", int(m_dataType), ").");
    }
}

void
MosaicMovieFile::checkFileGood(const std::string & inMessage) const
{
    if (!m_file.good())
    {
        ISX_THROW(ExceptionFileIO, inMessage + ": " + m_fileName);
    }
}

void
MosaicMovieFile::closeFileStream()
{
    if (isValid() && m_readOnly)
    {
        m_fileClosedForWriting = true;

        isx::closeFileStreamWithChecks(m_file, m_fileName);

        m_valid = false;
    }
}

uint64_t
MosaicMovieFile::readFrameTimestamp(const isize_t inIndex)
{
    const TimingInfo & ti = getTimingInfo();

    if (hasFrameTimestamps() && ti.isIndexValid(inIndex))
    {
        // The first pixel of the header should evaluate to 0x0A0 according
        // to the sensor spec, so check that in debug mode for sanity.
#ifndef NDEBUG
        seekForReadFrame(ti.timeIdxToRecordedIdx(inIndex), false, false);
        uint16_t sanityCheck = 0;
        m_file.read(reinterpret_cast<char *>(&sanityCheck), sizeof(sanityCheck));
        ISX_ASSERT(sanityCheck == 0x0A0);
#endif

        // The TSC bytes are spread across the last 8 pixels of the first line.
        // See sensor spec for more details.
        constexpr size_t tscOffset = 1272 * sizeof(uint16_t);
        std::array<uint16_t, 8> tscPixels;

        // We convert them to 4 roomy 64-bit components so that they can shifted and stitched
        // together into the final 64-bit TSC.
        std::array<uint64_t, 4> tscComps;

        seekForReadFrame(ti.timeIdxToRecordedIdx(inIndex), false, false);
        m_file.seekg(tscOffset, m_file.cur);
        ISX_LOG_DEBUG_FRAME_TIME("Reading TSC bytes from file location ", m_file.tellg());
        m_file.read(reinterpret_cast<char *>(&(tscPixels[0])), sizeof(tscPixels));
        if (!m_file.good())
        {
            ISX_THROW(ExceptionFileIO, "Error reading TSC ", inIndex);
        }

        // Deal with the 16bit->12bit->16bit encoding from register->sensor->file.
        for (size_t j = 0; j < tscComps.size(); ++j)
        {
            ISX_LOG_DEBUG_FRAME_TIME("Read TSC pixels ", 2*j, ", ", 2*j + 1, ": ", tscPixels[2*j], ", ", tscPixels[2*j + 1]);
            tscComps[j] = uint64_t((tscPixels[2*j + 1] << 4) | (tscPixels[2*j] >> 4));
            ISX_LOG_DEBUG_FRAME_TIME("Recovered TSC comp ", j, ": ", tscComps[j]);
        }

        // Shift and stitch the 16-bit components of the 64-bit timestamp together.
        const uint64_t tsc = (tscComps[3] << 48) | (tscComps[2] << 32) | (tscComps[1] << 16) | tscComps[0];
        ISX_LOG_DEBUG_FRAME_TIME("Recovered TSC ", tsc);
        return tsc;
    }

    return 0;
}

bool
MosaicMovieFile::hasFrameTimestamps() const
{
    if ((m_extraProperties != nullptr) && m_hasFrameHeaderFooter)
    {
        const auto producer = m_extraProperties.find("producer");
        if (producer != m_extraProperties.end())
        {
            const auto version = producer->find("version");
            if (version != producer->end())
            {
                return versionAtLeast(version->get<std::string>(), 1, 1, 1);
            }
        }
    }
    return false;
}

} // namespace isx
