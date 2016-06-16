#include "isxHdf5FileHandle.h" 
#include "isxProjectFile.h"
#include "isxLog.h"
#include <fstream>
namespace isx {

 
    class ProjectFile::Impl
    {
    public:
        /// Constructor
        ///
        Impl(std::string & inFileName) 
        {
            int openFlag = H5F_ACC_RDWR;
            if ( false == exists(inFileName))
            {
                openFlag = H5F_ACC_TRUNC;                
            }
        
            m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
            m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, openFlag);
                
            if(false == isInitialized())
            {
                // Initialize the data model if the file hasn't been initialized
                initDataModel();
            }
        }
        
        ~Impl()
        {
            m_file->close();

            m_file.reset();
            m_fileHandle.reset();
        }
         
        bool exists(std::string & inFileName);
        bool isInitialized();
        void initDataModel();
        
        /// \return Hdf5FileHandle
        ///
        SpHdf5FileHandle_t
        getHdf5FileHandle() const
        {
            return m_fileHandle;
        }
        
    private:

        SpH5File_t m_file;
        SpHdf5FileHandle_t m_fileHandle;
        
        H5::Group  m_grProject;
        H5::Group  m_grFileHeader;
        H5::Group  m_grHistory;
        H5::Group  m_grAnnotations;
        H5::Group  m_grSchedules;
        H5::Group  m_grCells;
        

    };


    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE
    ///////////////////////////////////////////////////////////////////////////////
    ProjectFile::ProjectFile(std::string & inFileName) 
    {
        m_pImpl.reset(new Impl(inFileName));
    }

    ProjectFile::~ProjectFile() 
    {
    } 

        
    SpHdf5FileHandle_t ProjectFile::getHdf5FileHandle()
    {
        return m_pImpl->getHdf5FileHandle();
    }

 

    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE IMPLEMENTATION
    ///////////////////////////////////////////////////////////////////////////////
 
    bool ProjectFile::Impl::isInitialized()
    {        
        bool bInitialized = false;
        
        try
        {
            m_file->openGroup("/MosaicProject");
            bInitialized = true;
        }

        catch (const H5::FileIException& error)
        {
	    ISX_LOG_ERROR("Failed to open H5 file.", error.getDetailMsg());
        }
        return bInitialized;
    }

 
    void ProjectFile::Impl::initDataModel()
    { 
        
        m_grProject    = m_file->createGroup("/MosaicProject");
        m_grFileHeader = m_file->createGroup("/MosaicProject/FileHeader");
        m_grSchedules  = m_file->createGroup("/MosaicProject/Schedules");
                
    }
 
 
    bool ProjectFile::Impl::exists(std::string & inFileName)
    {
        std::ifstream infile(inFileName.c_str());
        return infile.good();
    }
 
}
