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
        Impl() :
            m_bValid(false)
        {
        }

        /// Constructor
        ///
        Impl(const std::string & inFileName) :
            m_bValid(false)
        {
            // H5F_ACC_RDWR fails if the file doesn't exist. 
            int openFlag = H5F_ACC_RDWR;   
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
                
            initialize();
        }

        Impl(const std::string & inFileName, const std::string & inInputFileName) :
            m_bValid(false)
        {
            int openFlag = H5F_ACC_TRUNC;
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
            createDataModel(inInputFileName);
        }
        
        ~Impl()
        {
            m_file->close();

            m_file.reset();
            m_fileHandle.reset();
        }
                
        /// \return Hdf5FileHandle
        ///
        SpHdf5FileHandle_t
        getHdf5FileHandle() const
        {
            return m_fileHandle;
        }

        bool 
        isValid()
        {
            return m_bValid;
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
        void createDataModel(const std::string & inInputFileName);
        
        SpH5File_t m_file;
        SpHdf5FileHandle_t m_fileHandle;
        
        H5::Group  m_grProject;
        H5::Group  m_grFileHeader;
        H5::Group  m_grHistory;
        H5::Group  m_grAnnotations;
        H5::Group  m_grSeries;
        H5::Group  m_grCells;
        
        std::vector<SpMovieSeries_t> m_movieSeries;

        bool m_bValid;
        

    };
    
    
    
    
        
    
    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE
    ///////////////////////////////////////////////////////////////////////////////
    ProjectFile::ProjectFile()
    {
        m_pImpl.reset(new Impl());
    }

    ProjectFile::ProjectFile(const std::string & inFileName) 
    {
        m_pImpl.reset(new Impl(inFileName));
    }

    ProjectFile::ProjectFile(const std::string & inFileName, const std::string & inInputFileName)
    {
        m_pImpl.reset(new Impl(inFileName, inInputFileName));
    }

    ProjectFile::~ProjectFile() 
    {
    } 

        
    SpHdf5FileHandle_t 
    ProjectFile::getHdf5FileHandle()
    {
        return m_pImpl->getHdf5FileHandle();
    }

    bool
    ProjectFile::isValid()
    {
        return m_pImpl->isValid();
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

        m_grProject    = m_file->openGroup(projectPath);
        m_grFileHeader = m_file->openGroup(headerPath);
        m_grSeries  = m_file->openGroup(seriesPath);
            
        hsize_t nObjInGroup = m_grSeries.getNumObjs();
        if(nObjInGroup != 0)
        {
            m_movieSeries.resize(nObjInGroup);
            for (hsize_t rs(0); rs < nObjInGroup; ++rs)
            {
                std::string rs_name = seriesPath + "/" + m_grSeries.getObjnameByIdx(rs);
                m_movieSeries[rs].reset(new MovieSeries(getHdf5FileHandle(), rs_name));
            }
        }

        m_bValid = true;
        
    }
    
    
    void 
    ProjectFile::Impl::createDataModel(const std::string & inInputFileName)
    { 
        
        m_grProject    = m_file->createGroup(projectPath);
        m_grFileHeader = m_file->createGroup(headerPath);
        m_grSeries  = m_file->createGroup(seriesPath);

        // Add the name of the input file to the file header
        H5::DataSpace inputfile_dataspace = H5::DataSpace(H5S_SCALAR);
        H5::StrType strdatatype(H5::PredType::C_S1, 256); // of length 256 characters
        H5::Attribute inputfile_attribute = m_grFileHeader.createAttribute("Input File", strdatatype, inputfile_dataspace);
        inputfile_attribute.write(strdatatype, inInputFileName.c_str());

        m_bValid = true;
                
    }
 
 

}