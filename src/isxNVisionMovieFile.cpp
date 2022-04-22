
#include "isxNVisionMovieFile.h"
#include "isxJsonUtils.h"

#include <ostream>
#include <fstream>
#include <json.hpp>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

// Copied from isxCompressedMovieFile.cpp
#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

/// The code that is responsible for decoding video data is based on the following code:
/// 1. isxBehavMovieFile.cpp
/// This file demonstrates how to import ffmpeg and use its C API to read data from third party behavioral movies
/// The logic in this file is more complex than what's required to decode nVision video data since
/// nVision video data is stored in a MJPEG container so each frame in the video container is an I frame, (i.e., key frame)
/// This makes the logic for seeking a frame in the file much simpler than isxBehavMovieFile.cpp
/// For more info I frames see: https://en.wikipedia.org/wiki/Video_compression_picture_types
/// 2. https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
/// Much of the code in the initializeCodec and decodePacket are derived from this code.
/// This repo is also a great resource for learning about ffmpeg and how to use its C API: https://github.com/leandromoreira/ffmpeg-libav-tutorial

// set to 1 to turn on nVision movie debug logging
#if 0
#define ISX_NVISION_MOVIE_LOG_DEBUG(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_NVISION_MOVIE_LOG_DEBUG(...)
#endif

/// Calculates the pts (presentation time stamp) based on a frame index and stream info.
/// Copied from: https://stackoverflow.com/questions/39983025/how-to-read-any-frame-while-having-frame-number-using-ffmpeg-av-seek-frame 
/// Currently it's not possible to seek to a frame based on the frame number using ffmpeg.
/// Instead you must seek to a frame based on the PTS, which is why this function in necessary.
/// This assumes a constant fps. Since this is an MJPEG container, that assumption seems to work for now.
/// If the video container format changes, then this may not work.
int64_t frameToPts(AVStream* pavStream, size_t frame)
{
    return (int64_t(frame) * pavStream->r_frame_rate.den *  pavStream->time_base.den) / (int64_t(pavStream->r_frame_rate.num) * pavStream->time_base.num);
}

namespace isx
{

std::ostream& operator<<(std::ostream& os, const NVisionMovieFile::Header & header)
{
	os << "nVision movie file header:\n";
	os << "\tFile Version: " << header.m_fileVersion << std::endl;
	os << "\tHeader Size:: " << header.m_headerSize << std::endl;
	os << "\tStart Epoch (ms): " << header.m_epochMs << std::endl;
	os << "\tUTC Offset (min): " << header.m_utcOffset << std::endl;
	os << "\tNum Frames: " << header.m_numFrames << std::endl;
	os << "\tNum Drops: " << header.m_numDrops << std::endl;
	os << "\tVideo Offset: " << header.m_videoOffset << std::endl;
	os << "\tVideo Size: " << header.m_videoSize << std::endl;
	os << "\tMetadata Offset: " << header.m_metaOffset << std::endl;
	os << "\tMetadata Size: " << header.m_metaSize << std::endl;
	os << "\tSession Offset: " << header.m_sessionOffset << std::endl;
	os << "\tSession Size: " << header.m_sessionSize << std::endl;
	return os;
}

NVisionMovieFile::NVisionMovieFile(const std::string &inFileName)
{
	m_fileName = inFileName;

	initializeFileStream();
	readHeader();
	readMetadataSegment();
	readSessionSegment();

	initializeCodec();

	m_valid = true;
}

NVisionMovieFile::~NVisionMovieFile()
{
	avformat_close_input(&m_formatCtx);
	avcodec_free_context(&m_videoCodecCtx);
}

void
NVisionMovieFile::initializeCodec()
{
	ISX_NVISION_MOVIE_LOG_DEBUG("Initializing the container, codec, and protocols for nVision movie.");

	m_formatCtx = avformat_alloc_context();
	if (!m_formatCtx)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Could not allocate memory for AVFormatContext");
	}

	// Open the file with ffmpeg.
	// Specify the type and offset of the video container.
	auto infmt = av_find_input_format("mjpeg");
	AVDictionary *dict = nullptr;
    av_dict_set_int(&dict, "skip_initial_bytes", int64_t(m_header.m_videoOffset), 0);
    int avRetCode = avformat_open_input(&m_formatCtx, m_fileName.c_str(), infmt, &dict);
	if (avRetCode != 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to open movie file (", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode), " ", avRetCode);
	}
	ISX_NVISION_MOVIE_LOG_DEBUG("Found format of container: ", m_formatCtx->iformat->name, ".");
	
	ISX_NVISION_MOVIE_LOG_DEBUG("Finding stream info from format.");

	// Get information about the streams stored in this file
	avRetCode = avformat_find_stream_info(m_formatCtx, nullptr);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to get stream info from movie file ( ", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}

	// Find first video stream
	for (uint32_t index = 0; index < m_formatCtx->nb_streams; ++index)
	{
		if (m_formatCtx->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoStreamIndex = index;
			break;
		}
	}
	ISX_NVISION_MOVIE_LOG_DEBUG("Found first video stream: ", m_videoStreamIndex);
	
	if (m_videoStreamIndex == -1)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to find video stream: ", m_fileName);
	}

	ISX_NVISION_MOVIE_LOG_DEBUG("Finding the proper decoder (codec)\n");
	AVCodecParameters *pCodecParams = m_formatCtx->streams[m_videoStreamIndex]->codecpar;
	AVCodec *pCodec = avcodec_find_decoder(pCodecParams->codec_id);

	if (pCodec == NULL)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to find supported codec: ", m_fileName);
	}
	ISX_NVISION_MOVIE_LOG_DEBUG("Codec: ", pCodec->name, " ID: ", pCodec->id, " Bit Rate: ", pCodecParams->bit_rate);

	m_videoCodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_videoCodecCtx)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to allocated memory for AVCodecContext");
	}

	// Fill the codec context based on the values from the supplied codec parameters
	avRetCode = avcodec_parameters_to_context(m_videoCodecCtx, pCodecParams);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to copy codec params to codec context from movie file: (", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	avRetCode = avcodec_open2(m_videoCodecCtx, pCodec, NULL);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to open codec through from movie file: (", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}
}

void
NVisionMovieFile::initializeFileStream()
{
	std::ios_base::openmode openmode = std::ios::binary | std::ios_base::in;
	m_file.open(m_fileName, openmode);
	if (!m_file.good() || !m_file.is_open())
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to open movie file for reading: ", m_fileName);
	}	
}

void
NVisionMovieFile::readHeader()
{
	m_file.seekg(0, std::ios_base::beg);
	m_file.read(reinterpret_cast<char *>(&m_header), sizeof(m_header));
	ISX_NVISION_MOVIE_LOG_DEBUG(m_header);
}

void
NVisionMovieFile::readMetadataSegment()
{
	std::unique_ptr<char[]> metadataStr(new char[m_header.m_metaSize + 1]);
	metadataStr[m_header.m_metaSize] = '\0';
	
	m_file.seekg(m_header.m_metaOffset, std::ios_base::beg);
	m_file.read(&(metadataStr[0]), m_header.m_metaSize);

	json metadata = json::parse(&(metadataStr[0]));

	// verify format of json metadata
	verifyJsonKey(metadata, "samples");

	if (metadata["samples"].size() != m_header.m_numFrames)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to parse per-frame json metadata. Unexpected number of samples: ", m_fileName);
	}

	verifyJsonKey(metadata["samples"][0], "tsc");
	verifyJsonKey(metadata["samples"][m_header.m_numFrames - 1], "tsc");

	verifyJsonKey(metadata["samples"][0], "fc");
	verifyJsonKey(metadata["samples"][m_header.m_numFrames - 1], "fc");

	const uint64_t startTsc = metadata["samples"][0]["tsc"];
	const uint64_t endTsc = metadata["samples"][m_header.m_numFrames - 1]["tsc"];
	const uint64_t durationUs = endTsc - startTsc;
	const uint64_t numSamplesWithin = metadata["samples"][m_header.m_numFrames - 1]["fc"].get<uint64_t>() - metadata["samples"][0]["fc"].get<uint64_t>();
	const uint64_t numSamples = m_header.m_numFrames + m_header.m_numDrops;

	std::vector<size_t> droppedFrames;
	if (metadata.find("droppedFrames") != metadata.end())
	{
		droppedFrames = metadata["droppedFrames"].get<std::vector<size_t>>();
	}

	m_timingInfos = {TimingInfo(
		Time(DurationInSeconds::fromMilliseconds(m_header.m_epochMs), static_cast<int32_t>(m_header.m_utcOffset) * 60),
		DurationInSeconds(durationUs, static_cast<size_t>(numSamplesWithin * 1e6)),
		numSamples,
		droppedFrames
	)};

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision movie timing info: ", m_timingInfos[0]);
}

void
NVisionMovieFile::readSessionSegment()
{
	std::string metadataStr;
	m_file.seekg(m_header.m_sessionOffset, std::ios_base::beg);
	std::getline(m_file, metadataStr, '\0');
	json metadata = json::parse(metadataStr);

	// verify format of json metadata
	verifyJsonKey(metadata, "cameraName");
	verifyJsonKey(metadata, "processingInterface");

	const std::string cameraName = metadata["cameraName"];
	verifyJsonKey(metadata["processingInterface"], cameraName);
	verifyJsonKey(metadata["processingInterface"][cameraName], "recordFov");
	verifyJsonKey(metadata["processingInterface"][cameraName]["recordFov"], "width");
	verifyJsonKey(metadata["processingInterface"][cameraName]["recordFov"], "height");
	verifyJsonKey(metadata["processingInterface"][cameraName]["recordFov"], "originx");
	verifyJsonKey(metadata["processingInterface"][cameraName]["recordFov"], "originy");

	json recordFov = metadata["processingInterface"][cameraName]["recordFov"];
	m_spacingInfo = SpacingInfo(
		SizeInPixels_t(recordFov["width"].get<uint64_t>(), recordFov["height"].get<uint64_t>()),
		SizeInMicrons_t(DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_SIZE),
		PointInMicrons_t(recordFov["originx"].get<uint64_t>(), recordFov["originy"].get<uint64_t>())
	);

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision movie spacing info: ", m_spacingInfo);
}

bool
NVisionMovieFile::isValid() const
{
	return m_valid;
}

SpVideoFrame_t
NVisionMovieFile::readFrame(isize_t inFrameNumber)
{
	const TimingInfo & ti = getTimingInfo();
	
	if (ti.isDropped(inFrameNumber))
	{
		SpVideoFrame_t frame = makeVideoFrame(inFrameNumber);
		std::memset(frame->getPixels(), 0, frame->getImageSizeInBytes());
		frame->setFrameType(VideoFrame::Type::DROPPED);
		return frame;
	}

	const size_t frameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
	ISX_NVISION_MOVIE_LOG_DEBUG("Input frame number, actual frame number on disk: ", inFrameNumber, " ", frameNumber);

	const int64_t seekPts = frameToPts(m_formatCtx->streams[m_videoStreamIndex], frameNumber);
	ISX_NVISION_MOVIE_LOG_DEBUG("Seek pts: ", seekPts);

	int avRetCode = av_seek_frame(m_formatCtx, m_videoStreamIndex, seekPts, AVSEEK_FLAG_ANY);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to seek to frame from movie file ( ", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}

	SpVideoFrame_t frame;
	AVPacket * pPacket = av_packet_alloc();
	if (!pPacket)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to allocated memory for AVPacket");
	}

	// Fill packet with data from stream
	while (av_read_frame(m_formatCtx, pPacket) >= 0)
	{
		// Decode packet if it's the video stream
		if (pPacket->stream_index == m_videoStreamIndex)
		{
			ISX_NVISION_MOVIE_LOG_DEBUG("Found packet with pts: ", pPacket->pts);
			frame = decodePacket(inFrameNumber, pPacket);
			break;
		}

		av_packet_unref(pPacket);
	}

	return frame;
}

SpVideoFrame_t
NVisionMovieFile::decodePacket(size_t inFrameNumber, AVPacket * pPacket)
{
	// Supply raw packet data as input to a decoder
	int avRetCode = avcodec_send_packet(m_videoCodecCtx, pPacket);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Error while sending a packet to the decoder from movie file ( ", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}

	AVFrame * pFrame = av_frame_alloc();
	if (!pFrame)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to allocated memory for AVFrame");
	}

	// Return decoded output data (into a frame) from a decoder
	avRetCode = avcodec_receive_frame(m_videoCodecCtx, pFrame);
	if (avRetCode != 0)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Error while receiving a frame from the decoder from movie file ( ", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
	}
	else
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Decoded frame(type=", av_get_picture_type_char(pFrame->pict_type), " size=", pFrame->pkt_size, " format=", pFrame->format, ")\n");

		// Check if the frame is a planar YUV 4:2:0.
		switch(pFrame->format)
		{
			case AV_PIX_FMT_YUV420P:
			case AV_PIX_FMT_YUYV422:
			case AV_PIX_FMT_YUV422P:
			case AV_PIX_FMT_YUV444P:
			case AV_PIX_FMT_YUV410P:
			case AV_PIX_FMT_YUV411P:
			case AV_PIX_FMT_YUVJ420P:
			case AV_PIX_FMT_YUVJ422P:
			case AV_PIX_FMT_YUVJ444P:
			case AV_PIX_FMT_YUV440P:
			case AV_PIX_FMT_YUVJ440P:
			case AV_PIX_FMT_YUVA420P:
				break;
			default:
			{
				ISX_THROW(isx::ExceptionFileIO, "Behavioral video playback, unsupported frame format: ", pFrame->format, ", ", m_fileName);
			}
		}

		// Verify size of decoded frame matches size of frame recorded in metadata
		const auto width = pFrame->linesize[0];
		const auto height = pFrame->height;
		if (static_cast<size_t>(width) != m_spacingInfo.getNumColumns() || static_cast<size_t>(height) != m_spacingInfo.getNumRows())
		{
			ISX_THROW(isx::ExceptionFileIO,
			"Dimensions of decoded frame (", width, " x ", height, ") do not match dimensions in metadata (", m_spacingInfo.getNumColumns(), " x ", m_spacingInfo.getNumRows(), ").");
		}

		// Read grayscale version of frame into memory
		// Images are stored in YUV color format. The Y channel is the luminance (i.e., brightness) of the image
		// The U and V channels are the color components which can be used to convert the image to RGB
		const Time t = getTimingInfo().convertIndexToStartTime(inFrameNumber);
		SpVideoFrame_t frame = makeVideoFrame(inFrameNumber);
		std::memcpy(frame->getPixels(), pFrame->data[0], width * height);
		av_frame_free(&pFrame);
		return frame;
	}
}

SpVideoFrame_t
NVisionMovieFile::makeVideoFrame(const isize_t inIndex) const
{
    return std::make_shared<VideoFrame>(
            getSpacingInfo(),
            m_spacingInfo.getNumPixels().getWidth() * getDataTypeSizeInBytes(m_dataType),
            1,
            getDataType(),
            getTimingInfo().convertIndexToStartTime(inIndex),
            inIndex);
}

const std::string &
NVisionMovieFile::getFileName() const
{
	return m_fileName;
}

const isx::TimingInfo &
NVisionMovieFile::getTimingInfo() const
{
	return m_timingInfos[0];
}

const isx::TimingInfos_t &
NVisionMovieFile::getTimingInfosForSeries() const
{
	return m_timingInfos;
}

const isx::SpacingInfo &
NVisionMovieFile::getSpacingInfo() const
{
	return m_spacingInfo;
}

DataType
NVisionMovieFile::getDataType() const
{
	return m_dataType;
}

} // namespace isx
