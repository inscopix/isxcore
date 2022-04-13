
#include "isxNVisionMovieFile.h"
#include "isxJsonUtils.h"

#include <ostream>
#include <fstream>
#include <json.hpp>

// set to 1 to turn on nVision movie debug logging
#if 0
#define ISX_NVISION_MOVIE_LOG_DEBUG(...) ISX_LOG_DEBUG(__VA_ARGS__)
#else
#define ISX_NVISION_MOVIE_LOG_DEBUG(...)
#endif

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
	os << "\tVideo Offseet: " << header.m_videoOffset << std::endl;
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

	m_valid = true;
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

	m_timingInfos = {TimingInfo(
		Time(DurationInSeconds::fromMilliseconds(m_header.m_epochMs), static_cast<int32_t>(m_header.m_utcOffset) * 60),
		DurationInSeconds(durationUs, static_cast<size_t>(numSamplesWithin * 1e6)),
		numSamples
	)};

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision timing info: ", m_timingInfos[0]);
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

	ISX_NVISION_MOVIE_LOG_DEBUG("nVision spacing info: ", m_spacingInfo);
}

bool
NVisionMovieFile::isValid() const
{
	return m_valid;
}

SpVideoFrame_t
NVisionMovieFile::readFrame(isize_t inFrameNumber)
{
	// TODO: decode compressed video data and return as video frame
	return getBlackFrame(inFrameNumber);
}

SpVideoFrame_t
NVisionMovieFile::getBlackFrame(isize_t inFrameNumber)
{
	Time t = getTimingInfo().convertIndexToStartTime(inFrameNumber);
	auto ret = std::make_shared<VideoFrame>(
		m_spacingInfo, m_spacingInfo.getNumPixels().getWidth(), 1, DataType::U8, t, inFrameNumber);
	std::memset(ret->getPixels(), 0, ret->getImageSizeInBytes());
	return ret;
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
