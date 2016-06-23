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
<<<<<<< HEAD
                for (isize_t m(0); m < nObjInGroup; ++m)
=======
                for (uint16_t m(0); m < nObjInGroup; ++m)
>>>>>>> develop
                {
                    std::string objName = MovieSeriesGroup.getObjnameByIdx(m);
                    std::string path = m_path + "/" + objName;
                    m_movies[m].reset(new Movie(m_fileHandle, path + "/Movie"));
                }
            }
        }
        
        /// Destructor
        ///
        ~Impl(){}
        
<<<<<<< HEAD
        isize_t 
        getNumMovies()
        {
            return m_movies.size();
        }
        
        SpMovie_t 
        getMovie(isize_t inIndex)
=======
        uint16_t 
        getNumMovies()
        {
            return (uint16_t)m_movies.size();
        }
        
        SpMovie_t 
        getMovie(uint16_t inIndex)
>>>>>>> develop
        {
            return m_movies[inIndex];
        }
        
        std::string 
        getName()
        {
            return m_path.substr(m_path.find_last_of("/")+1);
        }
        
        SpMovie_t 
<<<<<<< HEAD
        addMovie(const std::string & inName, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
=======
        addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight, isx::Ratio inFrameRate)
>>>>>>> develop
        {
            std::string path = m_path + "/" + inName;
            // Create the properties group
            // TODO sylvana 2016/06/14 : Movie needs to manage its properties and create/read the properties group in the file
            m_file->createGroup(path);
            m_file->createGroup(path + "/Properties");    
            m_movies.push_back(std::make_shared<Movie>(m_fileHandle, path + "/Movie", inNumFrames, inFrameWidth, inFrameHeight, inFrameRate));
            return m_movies[m_movies.size()-1];
        }
        
    private:
        SpH5File_t          m_file;
        SpHdf5FileHandle_t  m_fileHandle;
        std::string         m_path;
        std::vector<SpMovie_t>  m_movies;
        
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
        
<<<<<<< HEAD
    isize_t 
=======
    uint16_t 
>>>>>>> develop
    MovieSeries::getNumMovies()
    {
        return m_pImpl->getNumMovies();
    }
        
    SpMovie_t
<<<<<<< HEAD
    MovieSeries::getMovie(isize_t inIndex)
=======
    MovieSeries::getMovie(uint16_t inIndex)
>>>>>>> develop
    {
        return m_pImpl->getMovie(inIndex);
    }
    
    std::string 
    MovieSeries::getName()
    {
        return m_pImpl->getName();
    }
    
    SpMovie_t 
<<<<<<< HEAD
    MovieSeries::addMovie(const std::string & inName, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight, isx::Ratio inFrameRate)
=======
    MovieSeries::addMovie(const std::string & inName, size_t inNumFrames, size_t inFrameWidth, size_t inFrameHeight, isx::Ratio inFrameRate)
>>>>>>> develop
    {
        return m_pImpl->addMovie(inName, inNumFrames, inFrameWidth, inFrameHeight, inFrameRate);
    }
    
<<<<<<< HEAD
}
=======
}
>>>>>>> develop
