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

//    void
//    setCheckInCallback(AsyncCheckInCB_t inCheckInCB);

private:
    static constexpr uint16_t ISX_META_MAX_TILES = 1024; ///<The maximum number of tiles that can be used

#pragma pack(push, 1)
    /// Structs
    /* common descriptor header */
    struct desc_comp_header {
        uint16_t desc_type; /* isx_comp_desc_type */
        uint16_t is_comp; /* if video data is compressed */
        uint16_t comp_type; /* isx_video_comp_type or isx_meta_comp_type */
        uint16_t reserved; /* 32 bit packing */
    };

    /* common compression descriptor */
    struct isx_comp_desc {
        desc_comp_header header; /* compression header descriptor */

        uint32_t width; /* frame/tile width for data */
        uint32_t height; /* frame/tile height for data */

        uint64_t size; /* compressed/meta file size in bytes */
        uint64_t offset; /* data start offset in bytes from beginning of file */
    };

    /* isxd video file header */
    struct isx_comp_file_header {
        uint64_t secs_since_epoch_num; /* unix epoch time numerator */
        uint64_t secs_since_epoch_den; /* unix epoch time denominator */

        int32_t utc_offset; /* utc offset time */
        uint16_t file_format; /* file writer software version marker */
        uint16_t tile_count; /* tile count */

        uint64_t frame_count; /* video frame counter */

        isx_comp_desc frame; /* video frame compressed data */
        isx_comp_desc meta; /* meta data file information */

        uint64_t session_offset; /* session data offset */
        uint64_t session_size; /* session data size */
    };

    /* sensor meta data register values */
    struct isx_comp_sensor_meta_data {
        uint16_t led1_power; /* led 1 power */
        uint16_t led1_vf; /* led 1 forward voltage */
        uint16_t led2_power; /* led 2 power */
        uint16_t led2_vf; /* led 2 forward voltage */

        uint16_t efocus; /* efocus diopter */
        uint16_t reserved_1; /* reserved for future */
        uint16_t frame_counter;
        uint16_t reserved_2; /* reserved for 32 bit frame counter msb */

        uint64_t timestamp; /* 64 bit time stamp inserted by FPGA in frame */
    }; // 24 bytes

    /* frame meta data structure */
    struct isx_comp_frame_meta_data {
        isx_comp_sensor_meta_data meta; /* 12 short */
        uint8_t data[ISX_META_MAX_TILES]; /* actual size = tile_count for color */
    };
#pragma pack(pop)

    /* file descriptor type */
    enum isx_comp_desc_type {
        ISX_COMP_DESC_TYPE_NONE,
        ISX_COMP_DESC_TYPE_VIDEO,
        ISX_COMP_DESC_TYPE_META,
        ISX_COMP_DESC_TYPE_SESSION,
        ISX_COMP_DESC_TYPE_MAX
    };

    /* compression type for video frame data */
    enum isx_video_comp_type {
        ISX_VIDEO_COMP_TYPE_NONE,
        ISX_VIDEO_COMP_TYPE_H264,
        ISX_VIDEO_COMP_TYPE_VP8,
        ISX_VIDEO_COMP_TYPE_VP9,
        ISX_VIDEO_COMP_TYPE_H265,
        ISX_VIDEO_COMP_TYPE_CUSTOM,
        ISX_VIDEO_COMP_TYPE_MAX
    };

    /* compression type for meta data */
    enum isx_meta_comp_type {
        ISX_META_COMP_TYPE_NONE,
        ISX_META_COMP_TYPE_ZIP,
        ISX_META_COMP_TYPE_GZIP,
        ISX_META_COMP_TYPE_TZIP,
        ISX_META_COMP_TYPE_TAR,
        ISX_META_COMP_TYPE_CUSTOM,
        ISX_META_COMP_TYPE_MAX
    };

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The data type of the pixel values.
    DataType m_dataType = DataType::U16;

    /// The file stream
    std::fstream m_file;
//    std::fstream m_intermediate;

    /// The session size with indent == 4
    isize_t m_sessionSize = 0;

    /// The output writable movie.
    SpWritableMovie_t m_decompressedMovie;

    /// The libav parameters
    AVCodec *m_codec = nullptr; ///<The codec of the encoded stream
    AVCodecContext* m_decoderCtx = nullptr; ///<Decoder
    AVCodecParameters *m_decoderParameters = nullptr; ///Decoder's parameter
    AVFormatContext * m_formatCtx = nullptr; ///<Format decoder
    AVFrame* m_frame = nullptr; ///<The receiver of the frame
    AVPacket * m_packet = nullptr; ///<The packet send to decoder
    uint8_t m_videoStreamIndex = 0; ///<The stream index of the video. Our video should only get 1 stream (video).

    /// The header for the compressed movie file
    isx_comp_file_header m_header{};

    /// The size of metadata of each frame in bytes
    uint32_t m_frameMetaSize;

//    cv::VideoCapture m_vc;

    /// The extra properties to write in the JSON footer.
    json m_extraProperties = nullptr;

    /// The helper function to read both header and session footer
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
