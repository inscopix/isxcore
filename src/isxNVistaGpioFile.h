#ifndef ISX_NVISTA_GPIO_FILE_H
#define ISX_NVISTA_GPIO_FILE_H

#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxTimingInfo.h"
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A class that parses an nVista GPIO file
///
class NVistaGpioFile
{
public:


    /// Default contructor
    /// Constructs an invalid file object
    NVistaGpioFile();

    /// Constructor for a valid file object
    /// \param inFileName  the name of the file to read
    /// \param inOutputDir the directory that is going to contain the files for individual streams
    /// \param inCheckInCB check in callback used for Async processing
    NVistaGpioFile(const std::string & inFileName, const std::string & inOutputDir, AsyncCheckInCB_t inCheckInCB);

    /// Destructor
    ///
    ~NVistaGpioFile();

    /// \return id this is a valid object
    ///
    bool isValid();

    /// \return the file name for the GPIO original file
    ///
    const std::string & getFileName();

    /// Parses the original file and writes signals from different channels to a separate file
    /// \throw isx::ExceptionDataIO  if unrecognized packets are read from the file
    /// \throw isx::ExceptionFileIO  if there is a problem reading or writing files
    /// \return whether the process completed or it was cancelled
    AsyncTaskStatus parse();

    /// Get the ouput file name
    ///
    const std::string & getOutputFileName() const;

private:

    void initTimestamps();

    /// Writes event data (digital packets)
    AsyncTaskStatus writeLogicalFile();

    /// True if the movie file is valid, false otherwise.
    bool m_valid = false;

    SpHdf5FileHandle_t m_inputHandle;

    /// The name of the data file.
    std::string m_fileName;

    /// The directory where output files will be written.
    std::string m_outputDir;

    std::vector<double> m_timestamps;

    std::vector<uint8_t> m_counts;

    std::vector<std::vector<bool>> m_signals;

    std::string m_outputFileName;

    /// Check in callback for reporting progress
    AsyncCheckInCB_t m_checkInCB;

}; // class NVistaGpioFile

} // namespace isx

#endif // ISX_NVISTA_GPIO_FILE_H
