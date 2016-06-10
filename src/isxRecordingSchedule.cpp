#include "isxHdf5FileHandle.h" 
#include "isxRecordingSchedule.h"
#include "isxMovie.h"

namespace isx {

    class RecordingSchedule::Impl
    {
    public:  
    
        /// Default constructor
        /// 
        Impl(){}
        
        /// Constructor 
        /// \param inHdf5FileHandle file handle
        /// \param inPath path for the recording schedule
        ///
        Impl(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath):
        m_fileHandle(inHdf5FileHandle),
        m_path(inPath)
        {
            m_file = inHdf5FileHandle->get();
            H5::Group recordingScheduleGroup = m_file->openGroup(m_path);
            hsize_t nObjInGroup = recordingScheduleGroup.getNumObjs();
            
            if(nObjInGroup != 0)
            {
                m_movies.resize(nObjInGroup);
                
                // Initialize movies
                for (uint16_t m(0); m < nObjInGroup; ++m)
                {
                    std::string objName = recordingScheduleGroup.getObjnameByIdx(m);
                    std::string path = m_path + "/" + objName;
                    m_movies[m].reset(new Movie(m_fileHandle, path));
                }
            }
        }
        
        /// Destructor
        ///
        ~Impl(){}
        
        uint16_t 
        getNumMovies()
        {
            return (uint16_t)m_movies.size();
        }
        
        SpMovie_t 
        getMovie(uint16_t inIndex)
        {
            return m_movies[inIndex];
        }
        
        std::string 
        getName()
        {
            return m_path.substr(m_path.find_last_of("/")+1);
        }
        
        SpMovie_t 
        addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
        {
            std::string path = m_path + "/" + inName;
            m_movies.push_back(std::make_shared<Movie>(m_fileHandle, path, inNumFrames, inFrameWidth, inFrameHeight));
            return m_movies[m_movies.size()-1];
        }
        
    private:
        SpH5File_t          m_file;
        SpHdf5FileHandle_t  m_fileHandle;
        std::string         m_path;
        std::vector<SpMovie_t>  m_movies;
        
    };  

    //////////////////////////////////////////////////////////////////////////////
    //  RECORDING SCHEDULE
    ///////////////////////////////////////////////////////////////////////////////
    RecordingSchedule::RecordingSchedule()
    {
        
    }
    
    RecordingSchedule::RecordingSchedule(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
    {
        m_pImpl.reset(new Impl(inHdf5FileHandle, inPath));
    }
        
    RecordingSchedule::~RecordingSchedule()
    {
        
    }
        
    uint16_t 
    RecordingSchedule::getNumMovies()
    {
        return m_pImpl->getNumMovies();
    }
        
    SpMovie_t
    RecordingSchedule::getMovie(uint16_t inIndex)
    {
        return m_pImpl->getMovie(inIndex);
    }
    
    std::string 
    RecordingSchedule::getName()
    {
        return m_pImpl->getName();
    }
    
    SpMovie_t 
    RecordingSchedule::addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight)
    {
        return m_pImpl->addMovie(inName, inNumFrames, inFrameWidth, inFrameHeight);
    }
    
}