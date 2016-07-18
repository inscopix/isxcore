#include "isxProjectFile.h"
#include "isxLog.h"
#include "json.hpp"
#include <fstream>

namespace isx {
    

    // for convenience
    using json = nlohmann::json;

    class ProjectFile::Impl
    {
    public:
        Impl() :
            m_bValid(false)
        {
            initJson();
        }

        /// Constructor
        ///
        Impl(const std::string & inFileName) :
            m_filename(inFileName),
            m_bValid(false)
        {
            initJson();
            
            std::ifstream ifs;
            ifs.open(inFileName.c_str(), std::ifstream::in);
           
            // Read the json file if file exists
            if (ifs.good())
            {
                ifs >> m_fileContent;
                // TODO Read original filenames from datafiles
            }

            ifs.close();
            m_bValid = true;
        }
       
        ~Impl()
        {

            std::ofstream ofs;
            ofs.open(m_filename.c_str(), std::ofstream::trunc);

            // Write json using streams
            if (ofs.good())
            {
                ofs << m_fileContent.dump(4);
            }

            ofs.close();
        }
                
        bool 
        isValid()
        {
            return m_bValid;
        }
        
        isize_t
        getNumDataCollections()
        {
            json dataObj = m_fileContent["data"];
            isize_t nCollections = dataObj.size();
            return nCollections;
        }

        DataCollection
        getDataCollection(isize_t inIndex)
        {
            isize_t nCollections = getNumDataCollections();
            DataCollection dc;

            if (inIndex < nCollections)
            {
                json dataObj = m_fileContent["data"];
                json dataCollection = dataObj[inIndex];

                dc.name = dataCollection["name"].get<std::string>();

                json files = dataCollection["files"];
                               
                // iterate the array
                for (json::iterator it = files.begin(); it != files.end(); ++it)
                {
                    json fileObj = *it;
                    DataFileDescriptor fd;
                    fd.filename = fileObj["filename"].get<std::string>();
                    fd.type = (DataFileType)fileObj["data type"];
                    dc.files.push_back(fd);
                }

            }
            return dc;

        }

        void 
        addDataCollection(DataCollection & inData)
        {
            json dataCollection;
            json files;
            dataCollection["name"] = inData.name;

            for (isize_t f(0); f < inData.files.size(); ++f)
            {
                json fileObj;
                fileObj["filename"] = inData.files[f].filename;
                fileObj["data type"] = (int)inData.files[f].type;
                files.push_back(fileObj);
            }

            dataCollection["files"] = files;

            m_fileContent["data"].push_back(dataCollection);

            // TODO Update original filenames list
        }

        void 
        removeDataCollection(isize_t inCollectionIndex)
        {
            m_fileContent["data"].erase(inCollectionIndex);
            
            // TODO Update original filenames list
        }

        void 
        addFileToDataCollection(DataFileDescriptor & inFileDesc, isize_t inCollectionIndex)
        {
            json fileObj;
            fileObj["filename"] = inFileDesc.filename;
            fileObj["data type"] = (int)inFileDesc.type;
            m_fileContent["data"][inCollectionIndex]["files"].push_back(fileObj);
            
            // TODO Update original filenames list
        }

        void 
        removeFileFromDataCollection(isize_t inFileDescIndex, isize_t inCollectionIndex) 
        {
            m_fileContent["data"][inCollectionIndex]["files"].erase(inFileDescIndex);
            
            // TODO Update original filenames list
        }
        
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

        static const int fileVersionMajor = 1;
        static const int fileVersionMinor = 0;
     
        void initJson()
        {
            m_fileContent = json();
            m_fileContent["header"]["mosaic version"] = { CoreVersionMajor() , CoreVersionMinor(), CoreVersionBuild() };          
            m_fileContent["header"]["file version"] = { fileVersionMajor, fileVersionMinor };
            m_fileContent["data"] = json();

        }

        std::string m_filename;
        std::vector<std::string> m_originalFilenames;        
        json m_fileContent;

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


    ProjectFile::~ProjectFile() 
    {
        m_pImpl.reset();
    } 

        
    bool
    ProjectFile::isValid()
    {
        return m_pImpl->isValid();
    }

    
    isize_t
    ProjectFile::getNumDataCollections()
    {
        return m_pImpl->getNumDataCollections();
    }
    

    ProjectFile::DataCollection ProjectFile::getDataCollection(isize_t inIndex)
    {
        return m_pImpl->getDataCollection(inIndex);
    }


    void ProjectFile::addDataCollection(DataCollection & inData)
    {
        m_pImpl->addDataCollection(inData);
    }


    void ProjectFile::removeDataCollection(isize_t inCollectionIndex)
    {
        m_pImpl->removeDataCollection(inCollectionIndex);
    }


    void ProjectFile::addFileToDataCollection(DataFileDescriptor & inFileDesc, isize_t inCollectionIndex)
    {
        m_pImpl->addFileToDataCollection(inFileDesc, inCollectionIndex);
    }


    void ProjectFile::removeFileFromDataCollection(isize_t inFileDescIndex, isize_t inCollectionIndex)
    {
        m_pImpl->removeFileFromDataCollection(inFileDescIndex, inCollectionIndex);
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



}
