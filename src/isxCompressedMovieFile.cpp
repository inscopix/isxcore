#include "isxCompressedMovieFile.h"

#include <cstring>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
#include <opencv2/core.hpp>

#include "isxException.h"
#include "isxMovieFactory.h"
#include "isxPathUtils.h"

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

namespace isx
{

CompressedMovieFile::CompressedMovieFile ()
{
}

CompressedMovieFile::CompressedMovieFile (const std::string &inFileName, const std::string &outFileName)
{
    /// private members
    m_fileName = inFileName;
    m_decompressedMoviePath = outFileName;
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to open movie file for reading (", m_fileName, ")", " with error: ", getSystemErrorString());
    }
    // Read video header + session
    readVideoInfo();
    verifyVideoInfo();

    /// decoder
    int avRetCode;
    auto infmt = av_find_input_format("avi");
    AVDictionary *dict = nullptr;
    av_dict_set_int(&dict, "skip_initial_bytes", m_header.frame.offset, 0);
    avRetCode = avformat_open_input(&m_formatCtx, m_fileName.c_str(), infmt, &dict);
    if (avRetCode < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Decoder: Failed to open movie file ", m_fileName, " with error(", av_err2str(avRetCode), ")");
    }
    avRetCode = avformat_find_stream_info(m_formatCtx, nullptr);
    if (avRetCode < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Decoder: No stream exists in the movie file ", m_fileName, " with error(", av_err2str(avRetCode), ")");
    }
    // find first video stream and set (params + codec)
    int firstVideoStreamIndex = -1;
    for (uint32_t i = 0; i < m_formatCtx->nb_streams; ++i)
    {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_decoderParameters = m_formatCtx->streams[i]->codecpar;
            m_codec = avcodec_find_decoder(m_decoderParameters->codec_id);
            if (m_codec == nullptr)
            {
                ISX_THROW(isx::ExceptionFileIO, "Decoder: Cannot find the correct codec to decode file ", m_fileName);
            }
            firstVideoStreamIndex = i;
            break;
        }
    }
    if (firstVideoStreamIndex == -1)
    {
        ISX_THROW(isx::ExceptionFileIO,
                  "Failed to find video stream: ", m_fileName);
    }
    m_videoStreamIndex = firstVideoStreamIndex;
    // create codec context
    m_decoderCtx = avcodec_alloc_context3(m_codec);
    avRetCode = avcodec_parameters_to_context(m_decoderCtx, m_decoderParameters);
    if (avRetCode < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Decoder: Cannot convert codec parameters for file ", m_fileName,
                " with error(", av_err2str(avRetCode), ")");
    }
    avRetCode = avcodec_open2(m_decoderCtx, m_codec, nullptr);
    if (avRetCode < 0)
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Decoder: Cannot initialize the context for codec of file ", m_fileName,
                " with error(", av_err2str(avRetCode), ")");
    }

    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();
}

CompressedMovieFile::~CompressedMovieFile ()
{
    /// Free decoder allocation
    avCleanUp();

    /// Close file descriptors
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

void
CompressedMovieFile::avCleanUp ()
{
    ISX_LOG_DEBUG("AV clean up called");
    av_frame_free(&m_frame);
    av_packet_free(&m_packet);
    avformat_close_input(&m_formatCtx);
    avcodec_free_context(&m_decoderCtx);
}

void
CompressedMovieFile::readVideoInfo ()
{
    /// Header
    m_file.seekg(0, std::ios::beg);
    m_file.read((char *) &m_header, sizeof(m_header));
    ISX_ASSERT(m_header.tileCount >= 1 && m_header.tileCount <= ISX_META_MAX_TILES);
    ISX_ASSERT(m_header.pixelCount <= ISX_META_MAX_PIXELS);
    // sensor metadata (in 16 bit pixels) + tile data (each tile data is 8 bit int which is 1 byte)
    m_frameMetaSize = (sizeof(uint16_t) * m_header.pixelCount) + m_header.tileCount;

    /// Session json footer
    try
    {
        m_file.seekg(m_header.sessionOffset, std::ios::beg);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Cannot seek to session offset");
        }
        std::string jsonStr(m_header.sessionSize, '\0');
        m_file.read(&jsonStr[0], m_header.sessionSize);
        json session = json::parse(jsonStr);

        m_dataType = DataType(isize_t(session["dataType"]));
        auto type = DataSet::Type(size_t(session["type"]));
        if (!(type == DataSet::Type::MOVIE || type == DataSet::Type::IMAGE))
        {
            ISX_THROW(isx::ExceptionDataIO, "Expected type to be Movie or Image. Instead got ", size_t(type), ".");
        }
        m_timingInfo = convertJsonToTimingInfo(session["timingInfo"]);
        m_spacingInfo = convertJsonToSpacingInfo(session["spacingInfo"]);

        if (session.find("extraProperties") != session.end())
        {
            m_extraProperties = session["extraProperties"];
        }

        // Get the session size that matches how json is written in isxd file (writeJsonHeaderAtEnd)
        // 4 space indented json + endl + '\0' + session size at the end
        m_sessionSize = session.dump(4).length() + 2 + sizeof(isize_t);
        ISX_ASSERT(m_sessionSize >= m_header.sessionSize);
    }
    catch (const std::exception &error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing movie header.");
    }
}

void
CompressedMovieFile::verifyVideoInfo()
{
    /// Metadata
    uint64_t metadataExpectedSize =  m_frameMetaSize * m_timingInfo.getNumValidTimes();
    if (metadataExpectedSize != m_header.meta.size)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "File (", m_fileName, ") is corrupted: metadata size=", m_header.meta.size, "!" , metadataExpectedSize);
    }
    // This check might not be necessary.
    if (metadataExpectedSize + m_header.meta.offset != m_header.sessionOffset)
    {
        ISX_THROW(isx::ExceptionDataIO,
                  "File (", m_fileName, ") is corrupted: metadata size is not correct.");
    }
}

AsyncTaskStatus
CompressedMovieFile::readAllFrames (AsyncCheckInCB_t inCheckinCB)
{
    /// Check space, need information from the header and session from the file
    // Same as sufficientDiskSpace() from algo/isxOutputValidation since we don't have access to that function
    const isize_t compressedMovieSizeInBytes = getDecompressedFileSize();
    std::string rootDir;
    long long availableNumBytes = availableNumberOfBytesOnVolume(m_decompressedMoviePath, rootDir);
    bool partitionExists = availableNumBytes >= 0;
    ISX_ASSERT(partitionExists);

    if (isize_t(availableNumBytes) <= compressedMovieSizeInBytes)
    {
        const double availableGB = double(availableNumBytes) / std::pow(2, 30);
        const double requiredGB = double(compressedMovieSizeInBytes) / std::pow(2, 30);

        ISX_THROW(
            ExceptionUserInput,
            "You have insufficient disk space to write the output files.",
            "The outputs will require about ", requiredGB, " GB.",
            "We estimate you only have about ", availableGB, " GB in the destination partition. ");
    }
    SpWritableMovie_t decompressedOutputMovie = writeMosaicMovie(
        m_decompressedMoviePath, m_timingInfo, m_spacingInfo, m_dataType, m_header.pixelCount > 0);

    isize_t actualFrameIndex = 0;
    float progress;
    while (av_read_frame(m_formatCtx, m_packet) >= 0)
    {
        if (m_packet->stream_index == m_videoStreamIndex)
        {
            int response = avcodec_send_packet(m_decoderCtx, m_packet);
            while (response >= 0)
            {
                // Return decoded output data (into a frame) from a decoder
                response = avcodec_receive_frame(m_decoderCtx, m_frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    break;
                }
                else if (response < 0)
                {
                    ISX_THROW(
                        isx::ExceptionFileIO,
                        "Decoder: Failed to read frame ", actualFrameIndex,
                        " for file ", m_fileName, " with error(", av_err2str(response), ")");
                }

                // Our isxc file has metadata right after the video
                // libav decoder might treat the metadata as the frame
                // A hard stop is required to prevent metadata is treated as frames and read
                if (actualFrameIndex >= m_timingInfo.getNumTimes())
                {
                    break;
                }

                if (response >= 0)
                {
                    ISX_LOG_DEBUG("Frame[",
                                  m_decoderCtx->frame_number,
                                  "]: type=",
                                  av_get_picture_type_char(m_frame->pict_type));

                    // Convert AVFrame to OpenCV mat with 8 bit and grayscale
                    cv::Mat frame(
                        m_frame->height,
                        m_frame->width,
                        CV_8UC1,
                        m_frame->data[0],
                        m_frame->linesize[0]);

                    // Create result: OpenCV frame with 16 bit (used in isxd files)
                    cv::Mat resultFrame(frame.rows, frame.cols, CV_16U);

                    // Read metadata (frame's header pixels + mantissa)
                    m_file.seekg(m_header.meta.offset + (m_frameMetaSize * actualFrameIndex), std::ios::beg);
                    checkFileGood("Cannot locate metadata of frame=" + std::to_string(actualFrameIndex));
//                    ISX_LOG_DEBUG("frame[", actualFrameIndex, "]: open=", m_file.is_open(), " good=", m_file.good(), " tellg=", m_file.tellg());
                    std::vector<uint16_t> frameHeaderPixels(m_header.pixelCount);
                    if (m_header.pixelCount > 0)
                    {
                        m_file.read((char *) frameHeaderPixels.data(), m_header.pixelCount * sizeof(uint16_t));
                    }
                    std::vector<uint8_t> metadata(m_header.tileCount);
                    m_file.read((char *) metadata.data(), m_header.tileCount);

                    // Recover frame with metadata (8 -> 16 bit)
                    uint32_t tilePerLine = m_header.frame.width / m_header.meta.width;
                    for (uint32_t i = 0; i < m_header.tileCount; ++i)
                    {
                        // calc_m: m = 0(<=80), 1(<=112), 2(<=144), 3(<=176), 4(<=208)
                        uint8_t m = (metadata[i] - (81 - 32)) / 32;
                        uint8_t s = 4 - m;
//                        ISX_LOG_DEBUG("\ttile=", i, " m=", std::to_string(m), " s=", std::to_string(s));
                        cv::Rect tileRoi(
                            m_header.meta.width * (i % tilePerLine),
                            m_header.meta.height * (i / tilePerLine),
                            m_header.meta.width,
                            m_header.meta.height);
                        cv::Mat croppedRef(frame, tileRoi);
                        cv::Mat resultCroppedRef(resultFrame, tileRoi);

                        double alpha = std::pow(2, s);
                        croppedRef.convertTo(resultCroppedRef, CV_16U, alpha);
                    }

                    ISX_ASSERT(resultFrame.isContinuous());
                    ISX_ASSERT(resultFrame.type() == CV_16U);

                    SpVideoFrame_t outFrame = decompressedOutputMovie->makeVideoFrame(actualFrameIndex);
                    std::memcpy(outFrame->getPixels(), resultFrame.ptr(), outFrame->getImageSizeInBytes());
                    if (m_header.pixelCount > 0)
                    {
                        // Make output frame and write to file
                        // Re-create frame header (2 lines, hardcoded to ISX_START_PIXEL_IN_HEADER + 1) by:
                        // 1. insert 0s(= #col - sensor pixel count) in front of read sensor pixels => first line
                        // 2. insert 0s(= #col) at the back => second line
                        frameHeaderPixels.insert(
                            frameHeaderPixels.begin(),
                            (ISX_START_PIXEL_IN_HEADER + 1) - m_header.pixelCount,
                            0);
                        frameHeaderPixels.insert(
                            frameHeaderPixels.end(),
                            (ISX_START_PIXEL_IN_HEADER + 1),
                            0);
                        frameHeaderPixels[0] = 0x0A0;  // Bypass MosaicMovieFile::readFrameTimestamp sanity check
                        ISX_ASSERT(frameHeaderPixels.size() == ISX_FRAME_HEADER_FOOTER_SIZE);
                        std::vector<uint16_t> footer(ISX_FRAME_HEADER_FOOTER_SIZE, 0);
                        decompressedOutputMovie->writeFrameWithHeaderFooter(
                            frameHeaderPixels.data(),
                            outFrame->getPixelsAsU16(),
                            footer.data());
                    }
                    else
                    {
                        decompressedOutputMovie->writeFrame(outFrame);
                    }

                    actualFrameIndex += 1;

                    // Update progress
                    // This progress is not accurate if there are dropped frames
                    // A more accurate way is to add actualFrameIndex with all previous drops getting from timinginfo
                    // However, this will slow down the performance if a search is preformed too often
                    progress = float(actualFrameIndex) / float(m_timingInfo.getNumTimes());
                    if (inCheckinCB(progress))
                    {
                        // Close the file descriptor (file deletion is done at upper level function)
                        decompressedOutputMovie->closeForWriting();
                        return AsyncTaskStatus::CANCELLED;
                    }
                }
            }
        }
        av_packet_unref(m_packet);
    }
    decompressedOutputMovie->setExtraProperties(m_extraProperties.dump());
    decompressedOutputMovie->closeForWriting(m_timingInfo);

    return AsyncTaskStatus::COMPLETE;
}

std::string
CompressedMovieFile::getFileName () const
{
    return m_fileName;
}

const isx::TimingInfo &
CompressedMovieFile::getTimingInfo () const
{
    return m_timingInfo;
}

const isx::SpacingInfo &
CompressedMovieFile::getSpacingInfo () const
{
    return m_spacingInfo;
}

isize_t
CompressedMovieFile::getDecompressedFileSize (bool hasFrameHeaderFooter, isize_t bufferSize) const
{
    isize_t numPixels = getSpacingInfo().getTotalNumPixels();
    if (hasFrameHeaderFooter)
    {
        numPixels += ISX_FRAME_HEADER_FOOTER_SIZE * 2; // Header + Footer
    }
    const isize_t nFrames = getTimingInfo().getNumValidTimes();
    isize_t contentNumBytes = getDataTypeSizeInBytes(getDataType()) * nFrames * numPixels;

    isize_t calculatedSize = contentNumBytes + m_sessionSize + bufferSize;
    ISX_LOG_DEBUG("Decompressed file size=", contentNumBytes,
        " session=", m_sessionSize, " buffer=", bufferSize, " total=", calculatedSize);
    return calculatedSize;
}

DataType
CompressedMovieFile::getDataType() const
{
    return m_dataType;
}

void
CompressedMovieFile::checkFileGood(const std::string & inMessage) const
{
    if (!m_file.good())
    {
        ISX_THROW(isx::ExceptionFileIO, inMessage + ": " + m_fileName);
    }
}

} // namespace isx
