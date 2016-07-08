#include "isxHdf5FileHandle.h" 
#include "isxProjectFile.h"
#include "isxLog.h"

namespace isx {
    
    /* static */
    const std::string ProjectFile::projectPath = "/MosaicProject";
    /* static */
    const std::string ProjectFile::headerPath = "/MosaicProject/FileHeader";
    /* static */
    const std::string ProjectFile::seriesPath = "/MosaicProject/Series";
    /* static */
    const std::string ProjectFile::historyPath = "/MosaicProject/History";
    /* static */
    const std::string ProjectFile::annotationsPath = "/MosaicProject/Annotations";
    /* static */
    const std::string ProjectFile::cellsPath = "/MosaicProject/Cells";

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
            m_filename(inFileName),
            m_bValid(false)
        {
            // H5F_ACC_RDWR fails if the file doesn't exist. 
            int openFlag = H5F_ACC_RDWR;   
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
                
            initialize();
        }

        Impl(const std::string & inFileName, const std::vector<std::string> & inInputFileNames) :
            m_filename(inFileName),
            m_originalFilenames(inInputFileNames),
            m_bValid(false)
        {
            int openFlag = H5F_ACC_TRUNC;
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
            createDataModel();
        }
        
        ~Impl()
        {

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
        
        isize_t
        getNumMovieSeries()
        {
            return m_movieSeries.size();
        }
        
        SpMovieSeries_t 
        getMovieSeries(isize_t inIndex)
        {
            return m_movieSeries[inIndex];
        }
        
        SpMovieSeries_t addMovieSeries(const std::string & inName);


        std::string 
        getName()
        {
            return m_filename;
        }

        std::vector<std::string> & getOriginalNames()
        {
            return m_originalFilenames;
        }

    private:
        void initialize();
        void createDataModel();
        

        std::string m_filename;
        std::vector<std::string> m_originalFilenames;
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

    ProjectFile::ProjectFile(const std::string & inFileName, const std::vector<std::string> & inInputFileNames)
    {
        m_pImpl.reset(new Impl(inFileName, inInputFileNames));
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

    
    isize_t
    ProjectFile::getNumMovieSeries()
    {
        return m_pImpl->getNumMovieSeries();
    }
        

    SpMovieSeries_t 
    ProjectFile::getMovieSeries(isize_t inIndex)
    {
        return m_pImpl->getMovieSeries(inIndex);
    }
    
    SpMovieSeries_t 
    ProjectFile::addMovieSeries(const std::string & inName)
    {
        return m_pImpl->addMovieSeries(inName);
    }

    std::string 
    ProjectFile::getName()
    {
        return m_pImpl->getName();
    }

    std::vector<std::string> & 
    ProjectFile::getOriginalNames()
    {
        return m_pImpl->getOriginalNames();
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

        m_grProject     = m_file->openGroup(projectPath);
        m_grFileHeader  = m_file->openGroup(headerPath);
        m_grSeries      = m_file->openGroup(seriesPath);
        m_grHistory     = m_file->openGroup(historyPath);
        m_grAnnotations = m_file->openGroup(annotationsPath);
        m_grCells       = m_file->openGroup(cellsPath);

        // Read header
        H5::DataSet strDataset = m_grFileHeader.openDataSet("InputFiles");
        H5::DataSpace dataSpace = strDataset.getSpace();
        H5::DataType  dataType = strDataset.getDataType();
        hsize_t         strDims[1];
        dataSpace.getSimpleExtentDims(strDims);
        std::vector<const char*> namesC(strDims[0], NULL);        
        strDataset.read(namesC.data(), dataType);
        m_originalFilenames.resize(strDims[0]);
        for (isize_t i(0); i < strDims[0]; ++i)
        {
            m_originalFilenames[i] = namesC[i];
        }

            
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
    ProjectFile::Impl::createDataModel()
    { 
        
        m_grProject     = m_file->createGroup(projectPath);
        m_grFileHeader  = m_file->createGroup(headerPath);
        m_grSeries      = m_file->createGroup(seriesPath);
        m_grHistory     = m_file->createGroup(historyPath);
        m_grAnnotations = m_file->createGroup(annotationsPath);
        m_grCells       = m_file->createGroup(cellsPath);

        // Add the name of the input files to the file header
        std::vector<const char*> namesC;
        for (unsigned i = 0; i < m_originalFilenames.size(); ++i)
        {
            namesC.push_back(m_originalFilenames[i].c_str());
        }

        hsize_t         strDims[1] = { namesC.size() };
        H5::DataSpace   dataspace(1, strDims);

        // Variable length string
        H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE);
        H5::DataSet strDataset = m_grFileHeader.createDataSet("InputFiles", datatype, dataspace);
        strDataset.write(namesC.data(), datatype);

        m_bValid = true;
                
    }
 
 

}
