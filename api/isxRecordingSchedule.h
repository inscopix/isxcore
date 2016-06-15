#ifndef ISX_RECORDINGSCHEDULE_H
#define ISX_RECORDINGSCHEDULE_H

#include "isxMovie.h"
#include <vector>
#include <memory>



namespace isx {
    
       

    /// A class containing a recording schedule
    ///
    class RecordingSchedule
    {
    public:
    
        /// Default onstructor 
        /// Constructs an invalid recording schedule
        RecordingSchedule();
        
        
        /// Constructor 
        /// 
        RecordingSchedule(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath);
        
        /// Destructor
        ///
        ~RecordingSchedule();
        
        /// Get the number of movies in the recording schedule
        ///
        uint16_t getNumMovies();
        
        /// Get a movie by index
        ///
        SpMovie_t getMovie(uint16_t inIndex);
        
        /// Get recording schedule name
        ///
        std::string getName();
        
        /// Add a movie to the recording schedule
        ///
        SpMovie_t addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight, isx::Ratio inFrameRate);
        
    private:
        class Impl;
        /// Internal implementation of RecordingSchedule class
        ///
        std::unique_ptr<Impl> m_pImpl; 
        
    };
    
    
 
}
#endif // ISX_RECORDINGSCHEDULE_H