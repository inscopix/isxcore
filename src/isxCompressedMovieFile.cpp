#include "isxCompressedMovieFile.h"

#include <cstring>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
#include <opencv2/core.hpp>

#include "isxException.h"
#include "isxMovieFactory.h"


namespace isx
{

CompressedMovieFile::CompressedMovieFile()
{
}

CompressedMovieFile::CompressedMovieFile(const std::string & inFileName, const std::string & outFileName)
{
    /// private members
    m_fileName = inFileName;
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to open movie file for reading: ", m_fileName);
    }
    // Read video header + session
    readVideoInfo();
    m_decompressedMovie = writeMosaicMovie(outFileName, m_timingInfo, m_spacingInfo, m_dataType, false);

    /// decoder
    // create format context
    m_formatCtx = avformat_alloc_context();
    if (avformat_open_input(&m_formatCtx, m_fileName.c_str(), nullptr, nullptr) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Decoder: Failed to open movie file ", m_fileName);
    }
    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Decoder: No stream exists in the movie file ", m_fileName);
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
    if (avcodec_parameters_to_context(m_decoderCtx, m_decoderParameters) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Decoder: Cannot convert codec parameters for file ", m_fileName);
    }
    if (avcodec_open2(m_decoderCtx, m_codec, nullptr) < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Decoder: Cannot initialize the context for codec of file ", m_fileName);
    }

    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();
}

CompressedMovieFile::~CompressedMovieFile()
{
    /// Free decoder allocation
    avCleanUp();

    /// Close file descriptors
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

void
CompressedMovieFile::avCleanUp()
{
    ISX_LOG_DEBUG("AV clean up called");
    av_frame_free(&m_frame);
    av_packet_free(&m_packet);
    avformat_close_input(&m_formatCtx);
    avcodec_free_context(&m_decoderCtx);
}

void
CompressedMovieFile::readVideoInfo()
{
    /// Header
    m_file.seekg(0, std::ios::beg);
    m_file.read((char *)&m_header, sizeof(m_header));
    m_frameMetaSize = sizeof(CompSensorMetaData) + m_header.tileCount;

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
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing movie header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing movie header.");
    }
}

//void
//CompressedMovieFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
//{
//    m_checkInCB = inCheckInCB;
//}

AsyncTaskStatus
CompressedMovieFile::readAllFrames(AsyncCheckInCB_t inCheckinCB)
{
    isize_t actualFrameIndex = 0;
    float progress;
    while (av_read_frame(m_formatCtx, m_packet) >=0)
    {
        if (m_packet->stream_index == m_videoStreamIndex) {
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
                        "Decoder: Failed to read frame ", actualFrameIndex, " for file ", m_fileName);
                }

                // Our isxc file has metadata right after the video
                // libav decoder might treat the metadata as the frame
                // A hard stop is required to prevent extra broken frames being read
                if (actualFrameIndex >= m_timingInfo.getNumTimes())
                {
                    break;
                }

                if (response >= 0) {
                    ISX_LOG_DEBUG("Frame[", m_decoderCtx->frame_number, "]: type=", av_get_picture_type_char(m_frame->pict_type));

                    cv::Mat frame(m_frame->height, m_frame->width, CV_8UC1, m_frame->data[0], m_frame->linesize[0]);

                    // Recover frame with meta (8 -> 16 bit)
                    cv::Mat resultFrame(frame.rows, frame.cols, CV_16U);
                    m_file.seekg(m_header.meta.offset + m_frameMetaSize*actualFrameIndex, std::ios::beg);
                    checkFileGood("Cannot locate metadata of frame=" + std::to_string(actualFrameIndex));
//                    ISX_LOG_DEBUG("frame[", actualFrameIndex, "]: open=", m_file.is_open(), " good=", m_file.good(), " tellg=", m_file.tellg());
                    CompSensorMetaData meta {};
                    m_file.read((char*)&meta, sizeof(meta));
                    std::vector<uint8_t> data(m_header.tileCount);
                    m_file.read((char*)data.data(), m_header.tileCount);

                    uint32_t tilePerLine = m_header.frame.width / m_header.meta.width;
                    for (uint32_t i = 0; i < m_header.tileCount; ++i)
                    {
                        // calc_m: m = 0(<=80), 1(<=112), 2(<=144), 3(<=176), 4(<=208)
                        uint8_t m = (data[i] - (81-32)) / 32;
                        uint8_t s = 4 - m;
//                        ISX_LOG_DEBUG("\ttile=", i, " m=", std::to_string(m), " s=", std::to_string(s));
                        cv::Rect tileRoi(
                            m_header.meta.width * (i % tilePerLine),
                            m_header.meta.height * (i / tilePerLine),
                            m_header.meta.width,
                            m_header.meta.height);
                        cv::Mat croppedRef(frame, tileRoi);
                        cv::Mat resultCroppedRef(resultFrame, tileRoi);
//                        ISX_LOG_DEBUG("\t\tROI       : x=", tileRoi.x, " y=", tileRoi.y);

                        double alpha = std::pow(2, s);
                        croppedRef.convertTo(resultCroppedRef, CV_16U, alpha);
                    }

                    ISX_ASSERT(resultFrame.isContinuous()); // frame.type() == CV_16U
                    ISX_ASSERT(resultFrame.type() == CV_16U);
                    SpVideoFrame_t outFrame = m_decompressedMovie->makeVideoFrame(actualFrameIndex);
                    std::memcpy(outFrame->getPixels(), resultFrame.ptr(), outFrame->getImageSizeInBytes());
                    m_decompressedMovie->writeFrame(outFrame);

                    actualFrameIndex += 1;

                    // This progress is not accurate if there are dropped frames
                    // A more accurate way is to add actualFrameIndex with all previous drops getting from timinginfo
                    // However, this will slow down the performance if a search is preformed too often
                    progress = float(actualFrameIndex) / float(m_timingInfo.getNumTimes());
                    if (inCheckinCB(progress))
                    {
                        // Close the file descriptor (file deletion is done at upper level function)
                        m_decompressedMovie->closeForWriting();
                        return AsyncTaskStatus::CANCELLED;
                    }
                }
            }
        }
        av_packet_unref(m_packet);
    }
    m_decompressedMovie->setExtraProperties(m_extraProperties.dump());
    m_decompressedMovie->closeForWriting(m_timingInfo);

    return AsyncTaskStatus::COMPLETE;
}

std::string
CompressedMovieFile::getFileName() const
{
    return m_fileName;
}

const isx::TimingInfo &
CompressedMovieFile::getTimingInfo() const
{
    return m_timingInfo;
}

const isx::SpacingInfo &
CompressedMovieFile::getSpacingInfo() const
{
    return m_spacingInfo;
}

isize_t
CompressedMovieFile::getDecompressedFileSize(bool hasFrameHeaderFooter, isize_t bufferSize) const
{
    isize_t numPixels = getSpacingInfo().getTotalNumPixels();
    if (hasFrameHeaderFooter)
    {
        numPixels += ISX_FRAME_HEADER_FOOTER_SIZE;
    }
    const isize_t nFrames = getTimingInfo().getNumValidTimes();
    isize_t contentNumBytes = getDataTypeSizeInBytes(getDataType()) * nFrames * numPixels;

    isize_t calculatedSize = contentNumBytes + m_sessionSize + bufferSize;
    ISX_LOG_DEBUG("Decompressed file size=", calculatedSize);
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
