#ifndef ISX_MOVIESERIES_H
#define ISX_MOVIESERIES_H

#include "isxMovie.h"
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
        uint16_t getNumMovies();
        
        /// Get a movie by index
        ///
        SpMovie_t getMovie(uint16_t inIndex);
        
        /// Get recording series name
        ///
        std::string getName();
        
        /// Add a movie to the recording series
        ///
        SpMovie_t addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight, isx::Ratio inFrameRate);
        
    private:
        class Impl;
        /// Internal implementation of MovieSeries class
        ///
        std::unique_ptr<Impl> m_pImpl; 
        
    };
    
    
 
}
#endif // ISX_MOVIESERIES_H