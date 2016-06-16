#include "isxFileIO.h"
#include "isxRecording.h"
#include "isxProjectFile.h"
#include "isxHdf5FileHandle.h"

namespace isx
{
    class FileIO::Impl
    {
    public:
        Impl()
        {
            m_recordingFile.reset();
            m_mosaicProjectFile.reset();
        }
        ~Impl() {};
        
        
        void setInputFile(const std::string & inFileName)
        {
            m_filename = inFileName;

            // Figure out if the input is a recording from nVista or a Mosaic Project 
            std::vector<std::string> objInRoot;

            {
                SpH5File_t file = std::make_shared<H5::H5File>(inFileName.c_str(), H5F_ACC_RDONLY);
                SpHdf5FileHandle_t fileHandle = std::make_shared<Hdf5FileHandle>(file, H5F_ACC_RDONLY);
                fileHandle->getObjNames(objInRoot);
            }

            bool bMosaicProject = false;
            for (unsigned int obj(0); obj < objInRoot.size(); ++obj)
            {
                if (objInRoot.at(obj) == "MosaicProject")
                {
                    bMosaicProject = true;
                    break;
                }
            }

            if (bMosaicProject)
            {
                m_mosaicProjectFile.reset(new ProjectFile(m_filename));
            }
            else
            {
                std::string outputFileName = m_filename.substr(0, m_filename.size() - 5) + "_mp.hdf5";
                m_recordingFile.reset(new Recording(m_filename));
                m_mosaicProjectFile.reset(new ProjectFile(outputFileName));
            }

        }

        SpRecording_t recordingFile()
        {
            return m_recordingFile;
        }

        SpProjectFile_t mosaicProjectFile()
        {
            return m_mosaicProjectFile;
        }

    private:
        std::string m_filename;
        SpRecording_t m_recordingFile;
        SpProjectFile_t m_mosaicProjectFile;
    };



    FileIO::FileIO()
    {
        m_pImpl.reset(new Impl());
    }

    FileIO::~FileIO()
    {

    }

    void 
    FileIO::setInputFile(const std::string & inFileName)
    {
        m_pImpl->setInputFile(inFileName);
    }

    SpRecording_t 
    FileIO::recordingFile()
    {
        return m_pImpl->recordingFile();
    }

    SpProjectFile_t 
    FileIO::mosaicProjectFile()
    {
        return m_pImpl->mosaicProjectFile();
    }

}