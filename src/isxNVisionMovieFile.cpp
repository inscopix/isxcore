
#include "isxNVisionMovieFile.h"
#include "isxJsonUtils.h"
#include "isxMovie.h"

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
/// This makes the logic for seeking to a frame in the file much simpler than isxBehavMovieFile.cpp
/// For more info about I frames see: https://en.wikipedia.org/wiki/Video_compression_picture_types
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

NVisionMovieFile::NVisionMovieFile(
	const std::string & inFileName)
	: m_fileName(inFileName)
	, m_enableWrite(false)
{
	initializeFileStream();
	readHeader();
	readMetadata();
	initializeDecoder();

	m_valid = true;
}

NVisionMovieFile::NVisionMovieFile(
	const std::string & inFileName,
	const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
	: m_fileName(inFileName)
	, m_enableWrite(true)
	, m_timingInfos(TimingInfos_t{inTimingInfo})
    , m_spacingInfo(inSpacingInfo)
{
	if (!NVisionMovieFile::isResolutionSupported(m_spacingInfo))
	{
		ISX_THROW(isx::ExceptionUserInput, "Resolution (", m_spacingInfo.getNumPixels(), ") is unsupported for mjpeg encoder. Only resolutions with widths that are multiples of 64 are accepted.");
	}

	initializeFileStream();
	writeHeader();
	initializeEncoder();

	m_valid = true;
}

NVisionMovieFile::~NVisionMovieFile()
{
	if (m_formatCtx)
	{
		avformat_close_input(&m_formatCtx);
	}

	avcodec_free_context(&m_videoCodecCtx);

	if (m_avFrame)
	{
		av_frame_free(&m_avFrame);
	}
}

bool
NVisionMovieFile::isResolutionSupported(const SpacingInfo & inSpacingInfo)
{
	// The MJPEG encoder only supports video frames that have a width which is a multiple of 64.
	// Attempting to encode a video frame with a width that is not a multiple of 64 will not result in an error with the codec,
	// however the resulting movie will have a resolution different from the frame originally written to the file.
	// For now prevent users from writing movies of these unsupported resolutions.
	return inSpacingInfo.getNumPixels().getWidth() % 64 == 0;
}

void
NVisionMovieFile::initializeFileStream()
{
	ISX_NVISION_MOVIE_LOG_DEBUG("Initializing file stream.");
	auto openMode = std::ios::binary | std::ios_base::in;
	if (m_enableWrite)
	{
		openMode |= std::ios_base::out | std::ios_base::trunc;
	}
	m_file.open(m_fileName, openMode);
	if (!m_file.good() || !m_file.is_open())
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to open movie file for ", (m_enableWrite ? "writing" : "reading"), " (", m_fileName, ")");
	}
	ISX_NVISION_MOVIE_LOG_DEBUG("File stream initialized.");
}

void
NVisionMovieFile::initializeDecoder()
{
	ISX_NVISION_MOVIE_LOG_DEBUG("Initializing the container, decoder, and protocols for reading nVision movie.");

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
NVisionMovieFile::initializeEncoder()
{
	ISX_NVISION_MOVIE_LOG_DEBUG("Initializing the container, encoder, and protocols for writing nVision movie.");

	// Initialize the MJPEG encoder
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!codec)
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to find MJPEG codec.");
    }

    m_videoCodecCtx = avcodec_alloc_context3(codec);
    if (!m_videoCodecCtx)
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed to allocate video codec context.");
    }

	// Initialize parameters of the codec based on the user-specified timing and spacing info
	m_videoCodecCtx->width = int(m_spacingInfo.getNumColumns());
    m_videoCodecCtx->height = int(m_spacingInfo.getNumRows());
	m_videoCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	m_videoCodecCtx->time_base= av_make_q(int(m_timingInfos[0].getStep().getNum()), int(m_timingInfos[0].getStep().getDen()));

	// Calculate the bit rate of the movie
	{
		const size_t bitPerByte = 8;
		const size_t bytesPerPixel = getDataTypeSizeInBytes(DataType::U8);
		const size_t numPixels = m_spacingInfo.getTotalNumPixels();
		const double frameRate = double(m_videoCodecCtx->time_base.den) / double(m_videoCodecCtx->time_base.num);
		m_videoCodecCtx->bit_rate = int64_t(frameRate * double(numPixels) * double(bytesPerPixel) * double(bitPerByte));
	}

	// Try to open the encoder
	ISX_NVISION_MOVIE_LOG_DEBUG("Trying to open the encoder.");
	int avRetCode;
	avRetCode = avcodec_open2(m_videoCodecCtx, codec, NULL);
    if (avRetCode < 0)
    {
		ISX_THROW(isx::ExceptionFileIO, "Failed to open codec with ffmpeg error message: ", av_err2str(avRetCode));
    }

	// Allocate a frame which will be used to send uncompressed video data to the encoder
	ISX_NVISION_MOVIE_LOG_DEBUG("Encoder opened. Allocating buffer for av frame.");
	m_avFrame = av_frame_alloc();
    if (!m_avFrame)
    {
		ISX_THROW(isx::ExceptionFileIO, "Failed to allocate video frame.");
    }
    m_avFrame->format = m_videoCodecCtx->pix_fmt;
    m_avFrame->width  = m_videoCodecCtx->width;
    m_avFrame->height = m_videoCodecCtx->height;

    avRetCode = av_frame_get_buffer(m_avFrame, 32);
    if (avRetCode < 0)
    {
        ISX_THROW(isx::ExceptionFileIO, "Failed allocate frame with ffmpeg error message: ", av_err2str(avRetCode));
    }
}

void
NVisionMovieFile::readHeader()
{
	ISX_NVISION_MOVIE_LOG_DEBUG("Reading header");
	checkFileGood("Movie file is bad before seeking to header");
	m_file.seekg(0, std::ios_base::beg);
	checkFileGood("Failed to seek to header");
	m_file.read(reinterpret_cast<char *>(&m_header), sizeof(m_header));
	checkFileGood("Failed to read header");
	ISX_NVISION_MOVIE_LOG_DEBUG("Read header:", m_header);
}

void
NVisionMovieFile::writeHeader()
{
	m_header.m_fileVersion = m_fileVersion;
	m_header.m_headerSize = sizeof(m_header);
	m_header.m_epochMs = m_timingInfos[0].getStart().getSecsSinceEpoch().toMilliseconds();
	m_header.m_utcOffset = m_timingInfos[0].getStart().getUtcOffset() / 60;
	m_header.m_videoOffset = sizeof(m_header);
	ISX_NVISION_MOVIE_LOG_DEBUG("Writing header: ", m_header);

	checkFileGood("Movie file is bad before seeking to beginning of file to write header");
	m_file.seekp(0, std::ios_base::beg);
	checkFileGood("Failed to seek to beginning of file to write header");
	m_file.write(reinterpret_cast<char *>(&m_header), sizeof(m_header));
	checkFileGood("Failed to write header");
	ISX_NVISION_MOVIE_LOG_DEBUG("Header written.");
}

void
NVisionMovieFile::readMetadata()
{
	json frameMetadata, sessionMetadata;

	// Check to see if per-frame metadata exists in file, otherwise skip reading.
	if (m_header.m_metaSize)
	{
		// Read per-frame metadata section first
		ISX_NVISION_MOVIE_LOG_DEBUG("Reading per-frame metadata.");
		std::unique_ptr<char[]> frameMetadataStr(new char[m_header.m_metaSize + 1]);
		frameMetadataStr[m_header.m_metaSize] = '\0';
		
		checkFileGood("Movie file is bad before seeking to frame metadata.");
		m_file.seekg(m_header.m_metaOffset, std::ios_base::beg);
		checkFileGood("Failed to seek to frame metadata.");
		m_file.read(&(frameMetadataStr[0]), m_header.m_metaSize);
		checkFileGood("Failed to read frame metadata.");
		ISX_NVISION_MOVIE_LOG_DEBUG("Read per-frame metadata: ", &(frameMetadataStr[0]));

		try
		{
			frameMetadata = json::parse(&(frameMetadataStr[0]));
		}
		catch (...)
		{
			ISX_THROW(isx::ExceptionUserInput, "Failed to parse json per-frame metadata for reading: ", &(frameMetadataStr[0]));
		}
	}
	else
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("No per-frame metadata, skipping to session metadata.");
		checkFileGood("Movie file is bad before seeking to frame metadata.");
		m_file.seekg(m_header.m_sessionOffset, std::ios_base::beg);
	}

	// Read the session metadata next
	ISX_ASSERT(m_header.m_sessionSize);
	ISX_NVISION_MOVIE_LOG_DEBUG("Reading session metadata.");
	std::unique_ptr<char[]> sessionMetadataStr(new char[m_header.m_sessionSize + 1]);
	sessionMetadataStr[m_header.m_sessionSize] = '\0';

	m_file.read(&(sessionMetadataStr[0]), m_header.m_sessionSize);
	checkFileGood("Failed to read session metadata.");
	ISX_NVISION_MOVIE_LOG_DEBUG("Read session metadata: ", &(sessionMetadataStr[0]));

	try
	{
		sessionMetadata = json::parse(&(sessionMetadataStr[0]));
	}
	catch (...)
	{
		ISX_THROW(isx::ExceptionUserInput, "Failed to parse json session metadata for reading: ", &(sessionMetadataStr[0]));
	}

	// If there is per-frame metadata, save it to memory
	if (frameMetadata.find("samples") != frameMetadata.end())
	{
		if (frameMetadata["samples"].size() != m_header.m_numFrames)
		{
			ISX_THROW(isx::ExceptionFileIO,
				"Failed to parse per-frame json metadata. Unexpected number of samples: ", m_fileName);
		}

		// Save the per-frame metadata in memory.
		// TODO: Optimize memory usage of per-frame metadata.
		// Try to see if there's a way to retrieve this info on demand instead of storing it in memory first.
		m_frameMetadatas = std::vector<std::string>(m_header.m_numFrames);
		for (size_t i = 0; i < m_header.m_numFrames; i++)
		{
			m_frameMetadatas[i] = frameMetadata["samples"][i].dump();
		}
	}
	
	// Construct timing info based on per-frame metadata
	// Check to see if there's a timing info key that can be read directly from the session metadata,
	// otherwise construct timing info based on keys in per-frame metadata.
	if (sessionMetadata.find("timingInfo") == sessionMetadata.end())
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Constructing timing info from per-frame metadata");

		verifyJsonKey(frameMetadata, "samples");
		if (frameMetadata["samples"].size() != m_header.m_numFrames)
		{
			ISX_THROW(isx::ExceptionFileIO,
				"Number of frame timestamps in metadata (", frameMetadata["samples"].size(), ") does not match number of frames in movie (", m_header.m_numFrames, ").");
		}

		verifyJsonKey(frameMetadata["samples"][0], "tsc");
		verifyJsonKey(frameMetadata["samples"][m_header.m_numFrames - 1], "tsc");

		verifyJsonKey(frameMetadata["samples"][0], "fc");
		verifyJsonKey(frameMetadata["samples"][m_header.m_numFrames - 1], "fc");

		// Compute timing info based on per-frame metadata
		const uint64_t startTsc = frameMetadata["samples"][0]["tsc"];
		const uint64_t endTsc = frameMetadata["samples"][m_header.m_numFrames - 1]["tsc"];
		const uint64_t durationUs = endTsc - startTsc;
		const uint64_t numSamplesWithin = frameMetadata["samples"][m_header.m_numFrames - 1]["fc"].get<uint64_t>() - frameMetadata["samples"][0]["fc"].get<uint64_t>();
		const uint64_t numSamples = m_header.m_numFrames + m_header.m_numDrops;

		std::vector<size_t> droppedFrames;
		if (frameMetadata.find("droppedFrames") != frameMetadata.end())
		{
			droppedFrames = frameMetadata["droppedFrames"].get<std::vector<size_t>>();
		}

		m_timingInfos = {TimingInfo(
			Time(DurationInSeconds::fromMilliseconds(m_header.m_epochMs), static_cast<int32_t>(m_header.m_utcOffset) * 60),
			DurationInSeconds(durationUs, static_cast<size_t>(numSamplesWithin * 1e6)),
			numSamples,
			droppedFrames
		)};
	}
	else
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Constructing timing info from timing info key in session metadata");
		m_timingInfos = {convertJsonToTimingInfo(sessionMetadata.at("timingInfo"))};
		sessionMetadata.erase("timingInfo");
	}

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision movie timing info: ", m_timingInfos[0]);

	// Construct spacing info based on session metadata
	// Check to see if there's a spacing info key that can be read directly,
	// otherwise construct spacing info based on keys in processing interface of session metadata.
	if (sessionMetadata.find("spacingInfo") == sessionMetadata.end())
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Constructing spacing info from processing interface key in session metadata");
		verifyJsonKey(sessionMetadata, "cameraName");
		verifyJsonKey(sessionMetadata, "processingInterface");

		const std::string cameraName = sessionMetadata["cameraName"];
		verifyJsonKey(sessionMetadata["processingInterface"], cameraName);
		verifyJsonKey(sessionMetadata["processingInterface"][cameraName], "recordFov");
		verifyJsonKey(sessionMetadata["processingInterface"][cameraName]["recordFov"], "width");
		verifyJsonKey(sessionMetadata["processingInterface"][cameraName]["recordFov"], "height");
		verifyJsonKey(sessionMetadata["processingInterface"][cameraName]["recordFov"], "originx");
		verifyJsonKey(sessionMetadata["processingInterface"][cameraName]["recordFov"], "originy");

		json recordFov = sessionMetadata["processingInterface"][cameraName]["recordFov"];
		m_spacingInfo = SpacingInfo(
			SizeInPixels_t(recordFov["width"].get<uint64_t>(), recordFov["height"].get<uint64_t>()),
			SizeInMicrons_t(DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_SIZE),
			PointInMicrons_t(recordFov["originx"].get<uint64_t>(), recordFov["originy"].get<uint64_t>())
		);
	}
	else
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Constructing spacing info from spacing info key in session metadata");
		m_spacingInfo = convertJsonToSpacingInfo(sessionMetadata["spacingInfo"]);
		sessionMetadata.erase("spacingInfo");
	}

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision movie spacing info: ", m_spacingInfo);

	m_extraProperties = sessionMetadata.dump();
}

void
NVisionMovieFile::writeMetadata()
{
	json frameMetadata;
	if (m_frameMetadatas.size())
	{
		if (m_frameMetadatas.size() != m_timingInfos[0].getNumValidTimes())
		{
			ISX_THROW(isx::ExceptionUserInput, "Number of frames in video does not match number of per-frame metadata.");
		}

		// Construct per-frame metadata based on data saved in class and write to file.
		frameMetadata = {{"samples", json::array()}};
		for (const auto & s : m_frameMetadatas)
		{
			frameMetadata["samples"].push_back(json::parse(s));
		}
		frameMetadata["droppedFrames"] = m_timingInfos[0].getDroppedFrames();
	}

	m_header.m_numFrames = m_timingInfos[0].getNumValidTimes();
	m_header.m_numDrops = m_timingInfos[0].getDroppedCount();

	const std::string frameMetadataStr = frameMetadata.dump();
	ISX_NVISION_MOVIE_LOG_DEBUG("Writing per-frame metadata: ", frameMetadataStr);
	m_header.m_metaOffset = m_header.m_videoOffset + m_header.m_videoSize;
	m_header.m_metaSize = frameMetadataStr.length();
	checkFileGood("Movie file is bad before seeking to frame metadata");
	m_file.seekp(m_header.m_metaOffset, std::ios_base::beg);
	checkFileGood("Failed to seek to frame metadata");
	m_file.write(frameMetadataStr.c_str(), m_header.m_metaSize);
	checkFileGood("Failed to write frame metadata");

	// Construct session metadata based on data saved in class and write to file.
	json sessionMetadata = json::parse(m_extraProperties);
	// Save the spacing info to the output file in case the resolution of the data
	// has been modified from the original acquisition settings in the session metadata.
	sessionMetadata["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);

	// Save the spacing info to the output file in case the resolution of the data
	// has been modified from the original acquisition settings in the session metadata.
	sessionMetadata["timingInfo"] = convertTimingInfoToJson(m_timingInfos[0]);

	const std::string sessionMetadataStr = sessionMetadata.dump();
	ISX_NVISION_MOVIE_LOG_DEBUG("Writing session metadata: ", sessionMetadataStr);
	m_header.m_sessionOffset = m_header.m_metaOffset + m_header.m_metaSize;
	m_header.m_sessionSize = sessionMetadataStr.length();
	m_file.write(sessionMetadataStr.c_str(), m_header.m_sessionSize);
	checkFileGood("Failed to write session metadata");
}

bool
NVisionMovieFile::isValid() const
{
	return m_valid;
}

SpVideoFrame_t
NVisionMovieFile::readFrame(isize_t inFrameNumber)
{
	if (m_enableWrite)
	{
		ISX_THROW(isx::ExceptionUserInput, "Cannot simultaneously read and write to nVision movies.");
	}

	const TimingInfo & ti = getTimingInfo();

	if (inFrameNumber >= ti.getNumTimes())
	{
		ISX_THROW(ExceptionUserInput, "Failed to read frame from file. Index is out of bounds.");
	}

	if (ti.isDropped(inFrameNumber))
	{
		SpVideoFrame_t frame = makeVideoFrame(inFrameNumber);
		std::memset(frame->getPixels(), 0, frame->getImageSizeInBytes());
		frame->setFrameType(VideoFrame::Type::DROPPED);
		return frame;
	}

	const size_t frameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
	ISX_NVISION_MOVIE_LOG_DEBUG("Input frame number, actual frame number on disk: ", inFrameNumber, " ", frameNumber);

	// Compute pts based on requested frame number
	const int64_t requestedPts = frameToPts(m_formatCtx->streams[m_videoStreamIndex], frameNumber);
	ISX_NVISION_MOVIE_LOG_DEBUG("Requested pts: ", requestedPts);

	SpVideoFrame_t frame;
	AVPacket * pPacket = av_packet_alloc();
	if (!pPacket)
	{
		ISX_THROW(isx::ExceptionFileIO,
			"Failed to allocated memory for AVPacket");
	}

	// Seeking to a frame with a specified pts is not guaranteed using the libavcodec API
	// This is known to occur when seeking near the end of a long movie and then seeking backwards to a random frame earlier in the movie
	// Sometimes multiple seek attempts are necessary in order to find the packet in the stream with the requested pts
	int64_t pts = AV_NOPTS_VALUE;
	int seekFlags = 0;
	int64_t seekPts = requestedPts;
	size_t seekFrameNumber = frameNumber;
	size_t numReads = 0;
	while (pts != requestedPts)
	{
		// If the requested frame is exactly one frame after the previous requested frame,
		// then the requested frame can be read directly from the stream without seeking
		if (frameNumber != (m_previousFrameNumber + 1))
		{
			// If the previous requested frame occurs after the requested frame, seek backwards
			if (m_previousFrameNumber > frameNumber)
			{
				seekFlags = AVSEEK_FLAG_BACKWARD;
			}

			int avRetCode = av_seek_frame(m_formatCtx, m_videoStreamIndex, seekPts, seekFlags);
			avcodec_flush_buffers(m_videoCodecCtx);
			if (avRetCode < 0)
			{
				ISX_THROW(isx::ExceptionFileIO,
					"Failed to seek to frame ", frameNumber, " from movie file ( ", m_fileName, ") with ffmpeg error message: ", av_err2str(avRetCode));
			}
		}

		// Read next frame in stream
		while (av_read_frame(m_formatCtx, pPacket) >= 0)
		{
			// Check if packet belongs to video stream
			if (pPacket->stream_index == m_videoStreamIndex)
			{
				numReads++;
				pts = pPacket->pts;
				ISX_NVISION_MOVIE_LOG_DEBUG("Found packet with pts: ", pts);

				// If the pts of the packet is less than the requested pts then
				// keep reading the next frame in the stream until the packet with the requested pts is found
				// If the the pts of the packet is greater than the requested pts, exit the loop and
				// retry seeking to a point in the stream before the requested pts
				if (pts >= requestedPts)
				{
					break;
				}
			}

			av_packet_unref(pPacket);
		}

		// Retry seeking to the requested pts by seeking backwards from the current position
		if (pts > requestedPts)
		{
			if (numReads == 1)
			{
				// This is the first time trying to seek backwards
				// Try to seek backwards with the requested pts
				ISX_ASSERT(seekPts == requestedPts);
				seekFlags = AVSEEK_FLAG_BACKWARD;
				ISX_NVISION_MOVIE_LOG_DEBUG("Failed to seek to frame ", frameNumber, " with pts ", requestedPts, ". Trying to seek backwards to requested frame.");
			}
			else
			{
				// This is not the first time trying to seek backwards
				// Decrement the seek frame number so that we can seek to a point in the stream before the requested pts
				seekFrameNumber--;
				seekPts = frameToPts(m_formatCtx->streams[m_videoStreamIndex], seekFrameNumber);
				ISX_NVISION_MOVIE_LOG_DEBUG("Failed to seek to frame ", frameNumber, " with pts ", requestedPts, ". Trying to seek backwards to frame ", seekFrameNumber, " with pts ", seekPts);
			}
		}
	}
	ISX_ASSERT(pts == requestedPts);
	if (numReads > 1)
	{
		ISX_NVISION_MOVIE_LOG_DEBUG("Found frame ", frameNumber, " with pts ", requestedPts, " after skipping ", numReads, " packets");
	}

	frame = decodePacket(inFrameNumber, pPacket);
	av_packet_unref(pPacket);

	m_previousFrameNumber = frameNumber;

	return frame;
}

void
NVisionMovieFile::writeFrame(const SpVideoFrame_t & inFrame)
{
	if (!m_enableWrite)
	{
		ISX_THROW(isx::ExceptionUserInput, "Cannot simultaneously read and write to nVision movies.");
	}
	
	// Write video data to frame buffer
	// Frame is formatted with YUV
	const auto width = m_videoCodecCtx->width;
	const auto height = m_videoCodecCtx->height;
	const auto & image = inFrame->getImage();
	const int imageLineSize = static_cast<int>(image.getRowBytes() / image.getPixelSizeInBytes());
	const uint8_t * pixels = image.getPixelsAsU8();
	
	// Write frame data in Y channel
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			m_avFrame->data[0][y * m_avFrame->linesize[0] + x] = pixels[y * imageLineSize + x];
		}
	}

	// Set blue and red chroma (Cb Cr) to 1 for the frame to make it black and white
	for (int y = 0; y < height / 2; y++)
	{
		for (int x = 0; x < width / 2; x++)
		{
			m_avFrame->data[1][y * m_avFrame->linesize[1] + x] = 128;
			m_avFrame->data[2][y * m_avFrame->linesize[2] + x] = 128;
		}
	}

	m_avFrame->pts = m_previousFrameNumber++;

	int avRetCode;
	avRetCode = avcodec_send_frame(m_videoCodecCtx, m_avFrame);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO, "Failed to send frame for encoding with ffmpeg error message: ", av_err2str(avRetCode));
	}

	AVPacket pkt {0};
	av_init_packet(&pkt);
	avRetCode = avcodec_receive_packet(m_videoCodecCtx, &pkt);
	if (avRetCode < 0)
	{
		ISX_THROW(isx::ExceptionFileIO, "Failed to receive encoded data with ffmpeg error message: ", av_err2str(avRetCode));
	}

	checkFileGood("Movie file is bad before writing video frame data");
	m_file.write(reinterpret_cast<const char *>(pkt.data), pkt.size);
	checkFileGood("Failed to write video frame data");
	m_header.m_videoSize += pkt.size;
	ISX_NVISION_MOVIE_LOG_DEBUG("Wrote frame ", inFrame->getFrameIndex(), " with encoded size of ", pkt.size, " bytes");
	av_packet_unref(&pkt);
}

std::string
NVisionMovieFile::readFrameMetadata(const isize_t inFrameNumber)
{
	const TimingInfo & ti = getTimingInfo();

	if (inFrameNumber >= ti.getNumTimes())
	{
		ISX_THROW(ExceptionUserInput, "Failed to read frame metadata from file. Index is out of bounds.");
	}

	if (ti.isIndexValid(inFrameNumber))
	{
		const size_t frameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
		return m_frameMetadatas[frameNumber];
	}

	return "null";
}

void
NVisionMovieFile::writeFrameMetadata(const std::string inFrameMetadata)
{
	const TimingInfo & ti = getTimingInfo();

	if (m_frameMetadatas.size() >= ti.getNumValidTimes())
	{
		ISX_THROW(ExceptionUserInput, "Failed to write frame metadata to file. Index is out of bounds.");
	}

	m_frameMetadatas.push_back(inFrameMetadata);
}

bool
NVisionMovieFile::hasFrameTimestamps() const
{
	return m_frameMetadatas.size() > 0;
}

uint64_t
NVisionMovieFile::readFrameTimestamp(const isize_t inFrameNumber)
{
	const TimingInfo & ti = getTimingInfo();

	if (inFrameNumber >= ti.getNumTimes())
	{
		ISX_THROW(ExceptionUserInput, "Failed to read frame timestamp from file. Index is out of bounds.");
	}

	if (hasFrameTimestamps() && ti.isIndexValid(inFrameNumber))
	{
		const size_t frameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
		const auto frameMetadata = json::parse(m_frameMetadatas[frameNumber]);
		verifyJsonKey(frameMetadata, "tsc");
		return frameMetadata.at("tsc").get<uint64_t>();
	}

	return 0;
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

void
NVisionMovieFile::checkFileGood(const std::string & inMessage) const
{
    if (!m_file.good())
    {
        ISX_THROW(ExceptionFileIO, inMessage, " (" + m_fileName, ")");
    }
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

std::string
NVisionMovieFile::getExtraProperties() const
{
	return m_extraProperties;
}

void
NVisionMovieFile::setExtraProperties(const std::string inExtraProperties)
{
	m_extraProperties = inExtraProperties;
}

void
NVisionMovieFile::closeForWriting()
{
	writeMetadata();
	writeHeader();
}

} // namespace isx
