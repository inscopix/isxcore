#include "isxHdf5FileHandle.h" 
#include "isxProjectFile.h"

namespace isx {

 
    class ProjectFile::Impl
    {
    public:
        /// Constructor
        ///
        Impl() {}
        bool open(std::string & inFileName);
        void close();
        
        bool exists(std::string & inFileName);
        bool isInitialized();
        void initDataModel();
        
    private:

        SpH5File_t m_file;
        SpHdf5FileHandle_t m_fileHandle;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE
    ///////////////////////////////////////////////////////////////////////////////
    ProjectFile::ProjectFile() :
        m_bIsOpen(false)
    {
        m_pImpl.reset(new Impl());
    }

    ProjectFile::~ProjectFile() 
    {

    }

    bool ProjectFile::open(std::string & inFileName) 
    {
        m_bIsOpen = m_pImpl->open(inFileName);

        return m_bIsOpen;
    }

    void ProjectFile::close() 
    {
        if (m_bIsOpen)
        {
            m_pImpl->close();
            m_bIsOpen = false;
        }
    }

 

    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE IMPLEMENTATION
    ///////////////////////////////////////////////////////////////////////////////
    bool ProjectFile::Impl::open(std::string & inFileName)
    {      
        int openFlag = H5F_ACC_RDWR;
        if ( false == exists(inFileName))
            openFlag = H5F_ACC_TRUNC;
        
        m_file.reset(new H5::H5File(inFileName.c_str(), openFlag));
        m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file);
                
        if(false == isInitialized())
        {
            // Initialize the data model if the file hasn't been initialized
            initDataModel();
        }
        return true;
    }


    bool ProjectFile::Impl::isInitialized()
    {        
        bool bInitialized = false;
        
        try
        {
            m_file->openGroup("/MosaicProject");
            bInitialized = true;
        }

        catch (H5::FileIException error)
        {
            // Do nothing
        }
        return bInitialized;
    }

    void ProjectFile::Impl::close()
    {
        m_file->close();

        m_file.reset();
        m_fileHandle.reset();
    }

    void ProjectFile::Impl::initDataModel()
    {
 
    }
 
 
    bool ProjectFile::Impl::exists(std::string & inFileName)
    {
        std::ifstream infile(fileName.c_str());
        return infile.good();
    }
 
}