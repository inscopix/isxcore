#include "isxHdf5FileHandle.h" 
#include "isxProjectFile.h"
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
                
            initialize();
        }
        
        ~Impl()
        {
            m_file->close();

            m_file.reset();
            m_fileHandle.reset();
        }
         
        bool exists(std::string & inFileName);
        
        /// \return Hdf5FileHandle
        ///
        SpHdf5FileHandle_t
        getHdf5FileHandle() const
        {
            return m_fileHandle;
        }
        
        uint16_t 
        getNumRecordingSchedules()
        {
            return (uint16_t)m_recordingSchedules.size();
        }
        
        SpRecordingSchedule_t 
        getRecordingSchedule(uint16_t inIndex)
        {
            return m_recordingSchedules[inIndex];
        }
        
        SpRecordingSchedule_t addRecordingSchedule(const std::string & inName);
        
    private:
        void initialize();
        void initDataModel();
        
        SpH5File_t m_file;
        SpHdf5FileHandle_t m_fileHandle;
        
        H5::Group  m_grProject;
        H5::Group  m_grFileHeader;
        H5::Group  m_grHistory;
        H5::Group  m_grAnnotations;
        H5::Group  m_grSchedules;
        H5::Group  m_grCells;
        
        std::vector<SpRecordingSchedule_t> m_recordingSchedules;
        

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

        
    SpHdf5FileHandle_t 
    ProjectFile::getHdf5FileHandle()
    {
        return m_pImpl->getHdf5FileHandle();
    }
    
    uint16_t 
    ProjectFile::getNumRecordingSchedules()
    {
        return m_pImpl->getNumRecordingSchedules();
    }
        

    SpRecordingSchedule_t 
    ProjectFile::getRecordingSchedule(uint16_t inIndex)
    {
        return m_pImpl->getRecordingSchedule(inIndex);
    }
    
    SpRecordingSchedule_t 
    ProjectFile::addRecordingSchedule(const std::string & inName)
    {
        return m_pImpl->addRecordingSchedule(inName);
    }

 

    ///////////////////////////////////////////////////////////////////////////////
    //  PROJECT FILE IMPLEMENTATION
    ///////////////////////////////////////////////////////////////////////////////
    
    SpRecordingSchedule_t 
    ProjectFile::Impl::addRecordingSchedule(const std::string & inName)
    {
        std::string path = "/MosaicProject/Schedules/" + inName;
        m_file->createGroup(path);        
        m_recordingSchedules.push_back(std::make_shared<RecordingSchedule>(m_fileHandle, path));
        return m_recordingSchedules[m_recordingSchedules.size() - 1];
    }
    
    void 
    ProjectFile::Impl::initialize()
    {
        // Get the number of objects and initialize schedules
        std::string rootObjName("/");
        H5::Group rootGroup = m_file->openGroup(rootObjName);
        hsize_t nObjInGroup = rootGroup.getNumObjs();
        
        if(nObjInGroup == 0)
        {
            initDataModel();
        }
        else
        {
            m_grProject    = m_file->openGroup("/MosaicProject");
            m_grFileHeader = m_file->openGroup("/MosaicProject/FileHeader");
            m_grSchedules  = m_file->openGroup("/MosaicProject/Schedules");
            
            nObjInGroup = m_grSchedules.getNumObjs();
            if(nObjInGroup != 0)
            {
                m_recordingSchedules.resize(nObjInGroup);
                for (hsize_t rs(0); rs < nObjInGroup; ++rs)
                {
                    std::string rs_name = "/MosaicProject/Schedules/" + m_grSchedules.getObjnameByIdx(rs);
                    m_recordingSchedules[rs].reset(new RecordingSchedule(getHdf5FileHandle(), rs_name));
                }
            }
            
        }
    }
    
    
    void 
    ProjectFile::Impl::initDataModel()
    { 
        
        m_grProject    = m_file->createGroup("/MosaicProject");
        m_grFileHeader = m_file->createGroup("/MosaicProject/FileHeader");
        m_grSchedules  = m_file->createGroup("/MosaicProject/Schedules");
                
    }
 
 
    bool 
    ProjectFile::Impl::exists(std::string & inFileName)
    {
        std::ifstream infile(inFileName.c_str());
        return infile.good();
    }
 
}