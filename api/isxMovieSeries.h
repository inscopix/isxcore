#ifndef ISX_MOVIESERIES_H
#define ISX_MOVIESERIES_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxRatio.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include <vector>
#include <memory>



namespace isx {
    
       

    /// A class containing a recording series
    ///
    class MovieSeries
    {
    public:
    
        /// Default onstructor 
        /// Constructs an invalid recording series
        MovieSeries();
        
        
        /// Constructor 
        /// 
        MovieSeries(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);
        
        /// Destructor
        ///
        ~MovieSeries();
        
        /// Get the number of movies in the recording series
        ///
        isize_t getNumMovies();
        
        /// Get a movie by index
        ///
        SpMovieInterface_t getMovie(isize_t inIndex);
        
        /// Get recording series name
        ///
        std::string getName();
        
        /// Add a movie to the recording series
        ///
        SpMovieInterface_t
        addMovie(const std::string & inName, const TimingInfo & inTimingInfo, const SpacingInfo & inSpacingInfo);
        
    private:
        class Impl;
        /// Internal implementation of MovieSeries class
        ///
        std::unique_ptr<Impl> m_pImpl; 
        
    };
    
    
 
}

#endif // ISX_MOVIESERIES_H
