#ifndef ISX_NVISTA_HDF5_MOVIE
#define ISX_NVISTA_HDF5_MOVIE

#include "isxMovie.h"
#include "isxCoreFwd.h"
#include "isxHdf5Movie.h"
#include "isxVideoFrame.h"
#include "isxMutex.h"

#include <memory>
#include <map>

namespace isx
{

/// Encapsulates a movie/recording from nVista stored in at least one HDF5 file.
///
class NVistaHdf5Movie : public Movie
                      , public std::enable_shared_from_this<NVistaHdf5Movie>
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid c++ object, but not a valid movie.
    NVistaHdf5Movie();

    /// Construct a new movie from one HDF5 dataset.
    ///
    /// \param inFileName name of movie file
    /// \param inHdf5FileHandle opaque HDF5 file handles
    /// \param inTimingInfo the timing info for the movie (from external source such as xml)
    /// \param inSpacingInfo the spacing info for the movie (from external source such as xml)
    NVistaHdf5Movie(const std::string &inFileName,
        const SpHdf5FileHandle_t & inHdf5FileHandle,
        const TimingInfo & inTimingInfo = TimingInfo(),
        const SpacingInfo & inSpacingInfo = SpacingInfo());

    /// Construct a new movie from vector of existing HDF5 datasets
    ///
    /// \param inFileName name of movie file
    /// \param inHdf5FileHandles vector of opaque HDF5 file handles
    /// \param inTimingInfo the timing info for the movie (from external source such as xml)
    /// \param inSpacingInfo the spacing info for the movie (from external source such as xml)
    NVistaHdf5Movie(const std::string &inFileName,
        const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles,
        const TimingInfo & inTimingInfo = TimingInfo(),
        const SpacingInfo & inSpacingInfo = SpacingInfo());

    /// Destructor
    ///
    ~NVistaHdf5Movie();

    // Overrides
    bool
    isValid() const override;

    void
    getFrame(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame) override;

    void
    getFrame(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame) override;

    void
    getFrameAsync(size_t inFrameNumber, MovieGetU16FrameCB_t inCallback) override;

    void
    getFrameAsync(size_t inFrameNumber, MovieGetF32FrameCB_t inCallback) override;

    void
    cancelPendingReads() override;

    const isx::TimingInfo &
    getTimingInfo() const override;

    const isx::SpacingInfo &
    getSpacingInfo() const override;

    DataType
    getDataType() const override;

    std::string
    getFileName() const override;

    void
    serialize(std::ostream& strm) const override;

private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The vector of pointers to HDF5 movie files.
    std::vector<std::unique_ptr<Hdf5Movie>> m_movies;

    /// The vector of cumulative frame indices.
    std::vector<isize_t> m_cumulativeFrames;
    
    std::string m_fileName;

    /// Handles most of the initialization.
    /// \param inFileName name of movie file
    /// \param inHdf5Files List of files containing the movie data
    /// \param inTimingInfo the timing info for the movie (from external source such as xml)
    /// \param inSpacingInfo the spacing info for the movie (from external source such as xml)
    void initialize(const std::string & inFileName,
        const std::vector<SpH5File_t> & inHdf5Files,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo);

    /// Initialize timing info from the HDF5
    ///
    void initTimingInfo(const std::vector<SpH5File_t> & inHdf5Files);

    /// Initialize timing info from the HDF5
    ///
    void initSpacingInfo(const std::vector<SpH5File_t> & inHdf5Files);

    /// Reads a uint16 frame directly from the associated file.
    ///
    /// The read occurs on whatever thread calls this function.
    void
    getFrameInternal(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame);

    /// Reads a float frame directly from the associated file.
    ///
    /// The read occurs on whatever thread calls this function.
    void
    getFrameInternal(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame);

    /// Read/infer the timing info of this from the HDF5 movie files.
    ///
    /// If some timing info is missing, this will fail and return false.
    ///
    /// \return     True if successful, false otherwise.
    bool
    readTimingInfo(std::vector<SpH5File_t> inHdf5Files);

    /// Read the spacing info of this from the HDF5 movie files.
    ///
    /// If some spacing info is missing, this will fail and return false.
    ///
    /// \return     True if successful, false otherwise.
    bool
    readSpacingInfo(std::vector<SpH5File_t> inHdf5Files);

    /// \return The HDF5 movie file index associated with a frame number.
    ///
    isize_t getMovieIndex(isize_t inFrameNumber);
    
    /// remove read request from our pending reads
    /// \param inReadRequestId Id of request to remove
    /// \return AsyncTaskHandle for the removed read request
    SpAsyncTaskHandle_t
    unregisterReadRequest(uint64_t inReadRequestId);
    
    uint64_t m_readRequestCount = 0;
    isx::Mutex m_pendingReadsMutex;
    std::map<uint64_t, SpAsyncTaskHandle_t> m_pendingReads;

    /// \return     The size of a pixel value in bytes.
    ///
    isize_t getPixelSizeInBytes() const;

    /// The shared implementation of getting different frame types synchronously.
    ///
    /// \param  inFrameNumber   The number of the frame to read.
    /// \param  outFrame        The shared pointer in which to store the frame data.
    template <typename FrameType>
    void getFrameTemplate(
            isize_t inFrameNumber,
            std::shared_ptr<FrameType> & outFrame);

    /// The shared implementation of getting different frame types asynchronously.
    ///
    /// \param  inFrameNumber   The number of the frame to read.
    /// \param  inCallback      The function used to return the retrieved video frame.
    template <typename FrameType>
    void getFrameAsyncTemplate(
            isize_t inFrameNumber,
            MovieGetFrameCB_t<FrameType> inCallback);
};

typedef std::shared_ptr<NVistaHdf5Movie>  SpNVistaHdf5Movie_t;
typedef std::weak_ptr<NVistaHdf5Movie>    WpNVistaHdf5Movie_t;

} // namespace isx

#endif // ifndef ISX_NVISTA_HDF5_MOVIE
