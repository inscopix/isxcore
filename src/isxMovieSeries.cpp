#include "isxHdf5FileHandle.h" 
#include "isxMovieSeries.h"
#include "isxMovie.h"

namespace isx {

    class MovieSeries::Impl
    {
    public:  
    
        /// Default constructor
        /// 
        Impl(){}
        
        /// Constructor 
        /// \param inHdf5FileHandle file handle
        /// \param inPath path for the recording series
        ///
        Impl(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath):
        m_fileHandle(inHdf5FileHandle),
        m_path(inPath)
        {
            m_file = inHdf5FileHandle->get();
            H5::Group MovieSeriesGroup = m_file->openGroup(m_path);
            hsize_t nObjInGroup = MovieSeriesGroup.getNumObjs();
            
            if(nObjInGroup != 0)
            {
                m_movies.resize(nObjInGroup);
                
                // Initialize movies
                for (isize_t m(0); m < nObjInGroup; ++m)
                {
                    std::string objName = MovieSeriesGroup.getObjnameByIdx(m);
                    std::string path = m_path + "/" + objName;
                    m_movies[m].reset(new Movie(m_fileHandle, path));
                }
            }
        }
        
        /// Destructor
        ///
        ~Impl(){}
        
        isize_t 
        getNumMovies()
        {
            return m_movies.size();
        }
        
        SpMovieInterface_t
        getMovie(isize_t inIndex)
        {
            return m_movies[inIndex];
        }
        
        std::string 
        getName()
        {
            return m_path.substr(m_path.find_last_of("/")+1);
        }
        
        SpMovieInterface_t
        addMovie(const std::string & inName, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
        {
            std::string path = m_path + "/" + inName;  
            m_movies.push_back(std::make_shared<Movie>(m_fileHandle, path, inNumFrames, inFrameWidth, inFrameHeight, inFrameRate));
            return m_movies[m_movies.size()-1];
        }
        
    private:
        SpH5File_t                      m_file;
        SpHdf5FileHandle_t              m_fileHandle;
        std::string                     m_path;
        std::vector<SpMovieInterface_t> m_movies;
        
    };  

    //////////////////////////////////////////////////////////////////////////////
    //  RECORDING SERIES
    ///////////////////////////////////////////////////////////////////////////////
    MovieSeries::MovieSeries()
    {
        
    }
    
    MovieSeries::MovieSeries(const SpHdf5FileHandle_t & inHdf5FileHandle, const std::string & inPath)
    {
        m_pImpl.reset(new Impl(inHdf5FileHandle, inPath));
    }
        
    MovieSeries::~MovieSeries()
    {
        
    }
        
    isize_t 
    MovieSeries::getNumMovies()
    {
        return m_pImpl->getNumMovies();
    }
        
    SpMovieInterface_t
    MovieSeries::getMovie(isize_t inIndex)
    {
        return m_pImpl->getMovie(inIndex);
    }
    
    std::string 
    MovieSeries::getName()
    {
        return m_pImpl->getName();
    }
    
    SpMovieInterface_t
    MovieSeries::addMovie(const std::string & inName, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
    {
        return m_pImpl->addMovie(inName, inNumFrames, inFrameWidth, inFrameHeight, inFrameRate);
    }
    
}
