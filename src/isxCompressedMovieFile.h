#ifndef ISX_COMPRESSED_MOVIE_FILE_H
#define ISX_COMPRESSED_MOVIE_FILE_H

#include <fstream>
#include <ios>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#include "isxAsync.h"
#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxJsonUtils.h"
#include "isxSpacingInfo.h"
#include "isxTimingInfo.h"
#include "isxVideoFrame.h"


namespace isx
{

/// Encapsulates movie information and data in a file.
///
/// All information is read from and written to one file.
/// The file stores the information or meta-data as a JSON formatted
/// string header.
/// The file stores the movie frame data in uncompressed binary form
/// after the header.
class CompressedMovieFile
{
public:
    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie file.
    CompressedMovieFile();

    /// Read constructor.
    ///
    /// This opens an existing movie file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the movie file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the movie file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
    CompressedMovieFile(const std::string & inFileName, const std::string & outFileName);

    /// Default destructor
    ///
    ~CompressedMovieFile();

    /// \return     The name of the file.
    ///
    std::string getFileName() const;

    /// \return     The timing information read from the movie.
    ///
    const isx::TimingInfo & getTimingInfo() const;

    /// \return     The spacing information read from the movie.
    ///
    const isx::SpacingInfo & getSpacingInfo() const;

    /// Calculate the size of the decompressed movie.
    /// \param hasFrameHeaderFooter   Whether the decompressed frame has header/footer (4 lines * 1280[uint16_t]pixels)
    /// \param bufferSize             The size of the extra buffer in case of the really full disk, default is 500 MB.
    /// \return                       The estimated size of the decompressed movie.
    ///
    isize_t getDecompressedFileSize(bool hasFrameHeaderFooter = true, isize_t bufferSize = 500000000) const;

    /// \return     The data type of a pixel value.
    ///
    DataType getDataType() const;
    AsyncTaskStatus readAllFrames(AsyncCheckInCB_t inCheckinCB);

private:
    ///
    /// Constants
    ///
    static constexpr uint16_t ISX_META_MAX_TILES = 1000;           ///< metadata max tiles count (32x32)
    static constexpr uint16_t ISX_META_MAX_PIXELS = 22;            ///< sensor header pixels
    static constexpr uint16_t ISX_FRAME_HEADER_FOOTER_SIZE = 2560; ///< number of pixels of header+footer of each frame
    static constexpr uint16_t ISX_START_PIXEL_IN_HEADER = 1279;    ///< starting pixel for sensor data in frame header

    ///
    /// Structs
    ///
    /// Need to read directly from the file, compacted
#pragma pack(push, 1)
    // common descriptor header
    struct DescCompHeader
    {
        uint16_t descType; ///< isx_comp_desc_type
        uint16_t isComp;   ///< if video data is compressed
        uint16_t compType; ///< isx_video_comp_type or isx_meta_comp_type
        uint16_t reserved; ///< 32 bit packing */
    }; // 8 bytes

    // common compression descriptor
    struct CompDesc
    {
        DescCompHeader header; ///< compression header descriptor

        uint32_t width;        ///< frame/tile width for data
        uint32_t height;       ///< frame/tile height for data

        uint64_t size;         ///< compressed/meta file size in bytes
        uint64_t offset;       ///< data start offset in bytes from beginning of file
    }; // 32 bytes

    // isxd video file header
    struct CompFileHeader
    {
        uint64_t secsSinceEpochNum; ///< unix epoch time numerator
        uint64_t secsSinceEpochDen; ///< unix epoch time denominator
        int64_t utcOffset;          ///< utc offset time

        uint64_t fileFormat;        ///< file writer software version marker
        uint64_t tileCount;         ///< actual tile count (<= ISX_META_MAX_TILES)
        uint64_t pixelCount;        ///< actual 2 bytes per pixel count in sensor meta data (<= ISX_META_MAX_PIXELS)
        uint64_t frameCount;        ///< video frame counter

        CompDesc frame;             ///< video frame compressed data
        CompDesc meta;              ///< meta data file information

        uint64_t sessionOffset;     ///< session data offset
        uint64_t sessionSize;       ///< session data size
    }; // 128 bytes

    // sensor meta data register values (placeholder, allocation is done with the CompFileHeader.pixelCount)
    struct CompSensorMetaData
    {
        uint16_t data[ISX_META_MAX_PIXELS];
    }; // max 44 bytes

    // frame meta data structure (placeholder, allocation is done with the CompFileHeader.tileCount)
    struct CompFrameMetaData
    {
        CompSensorMetaData meta;
        uint8_t data[ISX_META_MAX_TILES]; ///< actual size = tileCount for color
    }; // max 1044 (1000 + 44) bytes
#pragma pack(pop)

    ///
    /// enums
    ///
    // file descriptor type
    enum CompDescType {
        ISX_COMP_DESC_TYPE_NONE,
        ISX_COMP_DESC_TYPE_VIDEO,
        ISX_COMP_DESC_TYPE_META,
        ISX_COMP_DESC_TYPE_SESSION,
        ISX_COMP_DESC_TYPE_MAX
    };

    // compression type for video frame data
    enum VideoCompType {
        ISX_VIDEO_COMP_TYPE_NONE,
        ISX_VIDEO_COMP_TYPE_H264,
        ISX_VIDEO_COMP_TYPE_VP8,
        ISX_VIDEO_COMP_TYPE_VP9,
        ISX_VIDEO_COMP_TYPE_H265,
        ISX_VIDEO_COMP_TYPE_CUSTOM,
        ISX_VIDEO_COMP_TYPE_MAX
    };

    // compression type for meta data
    enum MetaCompType {
        ISX_META_COMP_TYPE_NONE,
        ISX_META_COMP_TYPE_ZIP,
        ISX_META_COMP_TYPE_GZIP,
        ISX_META_COMP_TYPE_TZIP,
        ISX_META_COMP_TYPE_TAR,
        ISX_META_COMP_TYPE_CUSTOM,
        ISX_META_COMP_TYPE_MAX
    };


    ///
    /// Members
    ///
    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The data type of the pixel values.
    DataType m_dataType = DataType::U16;

    /// The input file stream.
    std::fstream m_file;

    /// The session size with indent == 4
    isize_t m_sessionSize = 0;

    /// The output writable movie.
    SpWritableMovie_t m_decompressedMovie;

    /// The libav parameters
    AVCodec * m_codec = nullptr;                       ///< The codec of the encoded stream.
    AVCodecContext * m_decoderCtx = nullptr;           ///< Decoder.
    AVCodecParameters * m_decoderParameters = nullptr; ///< Decoder's parameter.
    AVFormatContext * m_formatCtx = nullptr;           ///< Format I/O context.
    AVFrame * m_frame = nullptr;                       ///< The receiver of the frame.
    AVPacket * m_packet = nullptr;                     ///< The packet send to decoder.
    uint8_t m_videoStreamIndex = 0;                    ///< The stream index of the video.
                                                       ///< isxd video should contain only 1 stream (video).

    /// The header for the compressed movie file
    CompFileHeader m_header{};

    /// The size of metadata of each frame in bytes
    isize_t m_frameMetaSize;

    /// The extra properties to write in the JSON footer.
    json m_extraProperties = nullptr;

    ///
    /// Functions
    ///
    /// The helper function to read both header and session footer
    ///
    void readVideoInfo();

    /// Clean up for libav allocations.
    ///
    void avCleanUp();

    /// Throw an exception is the file has gone bad.
    ///
    /// \param  inMessage   The message to prepend to the file name.
    void checkFileGood(const std::string & inMessage) const;
};

} // namespace isx

#endif // ISX_COMPRESSED_MOVIE_FILE_H
