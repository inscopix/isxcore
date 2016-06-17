#include "isxHdf5FileHandle.h" 
#include "isxProjectFile.h"
#include <fstream>
namespace isx {
    
    /* static */
    const std::string ProjectFile::projectPath = "/MosaicProject";
    /* static */
    const std::string ProjectFile::headerPath = "/MosaicProject/FileHeader";
    /* static */
    const std::string ProjectFile::seriesPath = "/MosaicProject/Series";

    class ProjectFile::Impl
    {
    public:
        /// Constructor
        ///
        Impl(const std::string & inFileName) 
        {
            // H5F_ACC_RDWR fails if the file doesn't exist. If it exists we open 
            // it with Read/Write permission and everything works. H5F_ACC_TRUNC 
            // opens it also with Read/Write permission but it creates the file if
            // it doesn't exist.This means that the file opening will always succeed,
            // whether you are opening an existing file or trying to create one from scratch
            int openFlag = H5F_ACC_RDWR;
            if ( false == exists(inFileName))
            {
                openFlag = H5F_ACC_TRUNC;                
            }
        
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
                
            initialize();
        }
        
        ~Impl()
        {
            m_file->close();

            m_file.reset();
            m_fileHandle.reset();
        }
         
        bool exists(const std::string & inFileName);
        
        /// \return Hdf5FileHandle
        ///
        SpHdf5FileHandle_t
        getHdf5FileHandle() const
        {
            return m_fileHandle;
        }
        
        uint16_t 
        getNumMovieSeries()
        {
            return (uint16_t)m_movieSeries.size();
        }
        
        SpMovieSeries_t 
        getMovieSeries(uint16_t inIndex)
        {
            return m_movieSeries[inIndex];
        }
        
        SpMovieSeries_t addMovieSeries(const std::string & inName);
        
    private:
        void initialize();
        void initDataModel();
        
        SpH5File_t m_file;
        SpHdf5FileHandle_t m_fileHandle;
        
        H5::Group  m_grProject;
        H5::Group  m_grFileHeader;
        H5::Group  m_grHistory;
        H5::Group  m_grAnnotations;
        H5::Group  m_grSeries;
        H5::Group  m_grCells;
        
        std::vector<SpMovieSeries_t> m_movieSeries;
        

    };
    
    
    
    
        
    
    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE
    ///////////////////////////////////////////////////////////////////////////////
    ProjectFile::ProjectFile(const std::string & inFileName) 
    {
        m_pImpl.reset(new Impl(inFileName));
    }

    ProjectFile::~ProjectFile() 
    {
    } 

        
    SpHdf5FileHandle_t 
    ProjectFile::getHdf5FileHandle()
    {
        return m_pImpl->getHdf5FileHandle();
    }
    
    uint16_t 
    ProjectFile::getNumMovieSeries()
    {
        return m_pImpl->getNumMovieSeries();
    }
        

    SpMovieSeries_t 
    ProjectFile::getMovieSeries(uint16_t inIndex)
    {
        return m_pImpl->getMovieSeries(inIndex);
    }
    
    SpMovieSeries_t 
    ProjectFile::addMovieSeries(const std::string & inName)
    {
        return m_pImpl->addMovieSeries(inName);
    }

 

    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE IMPLEMENTATION
    ///////////////////////////////////////////////////////////////////////////////
    
    SpMovieSeries_t 
    ProjectFile::Impl::addMovieSeries(const std::string & inName)
    {
        std::string path = seriesPath + "/" + inName;
        m_file->createGroup(path);        
        m_movieSeries.push_back(std::make_shared<MovieSeries>(m_fileHandle, path));
        return m_movieSeries[m_movieSeries.size() - 1];
    }
    
    void 
    ProjectFile::Impl::initialize()
    {
        // Get the number of objects and initialize series
        std::string rootObjName("/");
        H5::Group rootGroup = m_file->openGroup(rootObjName);
        hsize_t nObjInGroup = rootGroup.getNumObjs();
        
        if(nObjInGroup == 0)
        {
            initDataModel();
        }
        else
        {
            m_grProject    = m_file->openGroup(projectPath);
            m_grFileHeader = m_file->openGroup(headerPath);
            m_grSeries  = m_file->openGroup(seriesPath);
            
            nObjInGroup = m_grSeries.getNumObjs();
            if(nObjInGroup != 0)
            {
                m_movieSeries.resize(nObjInGroup);
                for (hsize_t rs(0); rs < nObjInGroup; ++rs)
                {
                    std::string rs_name = seriesPath + "/" + m_grSeries.getObjnameByIdx(rs);
                    m_movieSeries[rs].reset(new MovieSeries(getHdf5FileHandle(), rs_name));
                }
            }
            
        }
    }
    
    
    void 
    ProjectFile::Impl::initDataModel()
    { 
        
        m_grProject    = m_file->createGroup(projectPath);
        m_grFileHeader = m_file->createGroup(headerPath);
        m_grSeries  = m_file->createGroup(seriesPath);
                
    }
 
 
    bool 
    ProjectFile::Impl::exists(const std::string & inFileName)
    {
        std::ifstream infile(inFileName.c_str());
        return infile.good();
    }
 
}