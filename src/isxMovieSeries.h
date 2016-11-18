#ifndef ISX_MOVIE_SERIES_H
#define ISX_MOVIE_SERIES_H

#include "isxMovie.h"
#include "isxMosaicMovieFile.h"

#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace isx
{

/// Class for managing and interfacing with a series of movies, accurately tracking time gaps between movies.
/// ==> Also see GaplessMovieSeries for a class that ignores the time gaps between movies.
/// Movies in a MovieSeries have to
/// - have matching SpacingInfo
/// - have matching data- or frame rate
/// - have matching datatypes
/// - not be overlapping in time (at any point in time
///       at most one movie in the series has data)
///
/// The movies in a MovieSeries can have gaps in time, they
/// do not need to be consecutive in time. 
///
/// In addition to implementing the Movie interface (isxMovie.h)
/// a few methods allow retrieval of data in a way that's specific to 
/// MovieSeries.
///
class MovieSeries : public Movie
                  , public std::enable_shared_from_this<MovieSeries>
{
public:

    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid movie.
    MovieSeries();

    /// Read constructor.
    ///
    /// This creates a MovieSeries from a set of existing movie files.
    ///
    /// \param  inFileNames The vector containing the names of the movie files.
    ///
    /// \throw  isx::ExceptionFileIO    If reading any of the movie files fails.
    /// \throw  isx::ExceptionDataIO    If parsing any of the movie files fails.
    /// \throw  isx::ExceptionDataIO    If the givne movie files do not meet
    ///                                 the requirements of a MovieSeries.
    MovieSeries(const std::vector<std::string> & inFileNames);

    /// Write constructor.
    ///
    /// This creates a new MovieSeries with new individual Movie objects from a vector
    /// of file names and their metadata.
    ///
    /// \param  inFileNames     The names of the movie files.
    /// \param  inTimingInfo    The timing information of the movie.
    /// \param  inSpacingInfo   The spacing information of the movie.
    /// \param  inDataType      The pixel value data type.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the movie file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
    MovieSeries(const std::string & inFileNames,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo,
                DataType inDataType);

    /// get individual Movie objects
    const
    std::vector<SpMovie_t> &
    getMovies()
    const;

    // Overrides - see base classes for documentation
    bool
    isValid()
    const override;

    SpVideoFrame_t
    getFrame(isize_t inFrameNumber)
    override;

    void getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback) override;

    void cancelPendingReads() override;

    const TimingInfo & getTimingInfo() const override;

    const TimingInfos_t & getTimingInfosForSeries() const override;

    const SpacingInfo & getSpacingInfo() const override;

    DataType getDataType() const override;

    std::string getFileName() const override;

    void serialize(std::ostream & strm) const override;

private:
    
    std::pair<isize_t, isize_t>
    getMovieIndexAndFrameIndexFromGlobalFrameIndex(isize_t inFrameIndex) const;
    
    SpVideoFrame_t
    makeVideoFrameInternal(isize_t inIndex) const;

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;
    
    TimingInfo                          m_globalTimingInfo;
    TimingInfos_t                         m_timingInfos;
    SpacingInfo                         m_spacingInfo;

    std::vector<SpMovie_t>              m_movies;
    std::vector<isize_t>                m_moviesFirstFrameInGlobal;
};

} // namespace isx

#endif // def ISX_MOVIE_SERIES_H
