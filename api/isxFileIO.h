#ifndef ISX_FILEIO_H
#define ISX_FILEIO_H

#include <memory>
#include <string>

#include "isxCoreFwd.h"

namespace isx {
    /// A class to contain the input and output files
    ///
    class FileIO
    {
    public: 
        /// Constructor
        ///
        FileIO();
        
        /// Destructor
        ///
        ~FileIO();

        /// \param inFileName filename for the input file
        ///
        void setInputFile(const std::string & inFileName);
        
        /// \return the recording file 
        ///
        SpRecording_t recordingFile();
        
        /// \return the project file
        ///
        SpProjectFile_t mosaicProjectFile();


    private:
        /// Private implementation of FileIO
        ///
        class Impl;
        
        /// internal pointer to implementation
        ///
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif // ISX_FILEIO_H