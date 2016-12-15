#ifndef ISX_CELL_SET_FILE_H
#define ISX_CELL_SET_FILE_H

#include <fstream>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxImage.h"
#include "isxTrace.h"


namespace isx
{   
    
/// A class for a file containing all cells extracted from a movie
///
class CellSetFile
{
    
public:
    /// Empty constructor.
    ///
    /// This creates a valid c++ object but an invalid cell set file.
    CellSetFile();

    /// Read constructor.
    ///
    /// This opens an existing cell set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the cell set file.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSetFile(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the cell set file.
    /// \param  inTimingInfo    The timing information of the cell set.
    /// \param  inSpacingInfo   The spacing information of the cell set.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    CellSetFile(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo);

    /// Destructor.
    ///
    ~CellSetFile();
    
    /// \return True if the cell set file is valid, false otherwise.
    ///
    bool isValid() const; 

    /// \return     The name of the file.
    ///
    std::string getFileName() const;
    
    /// \return the number of cells contained in the file
    ///
    const isize_t numberOfCells();

    /// \return     The timing information read from the cell set.
    ///
    const isx::TimingInfo & getTimingInfo() const;

    /// \return     The spacing information read from the cell set.
    ///
    const isx::SpacingInfo & getSpacingInfo() const;
    
    /// Read cell data for cell ID. 
    /// \param inCellId the cell of interest
    /// \return a shared pointer to the trace data for the input cell. 
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    SpFTrace_t readTrace(isize_t inCellId);
    
    /// \return a shared pointer to the segmentation image for the input cell
    /// \param inCellId the cell of interest
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    SpImage_t readSegmentationImage(isize_t inCellId);
    
    /// Write cell data
    /// \param inCellId the cell of interest
    /// \param inSegmentationImage the image to write
    /// \param inData the trace to write
    /// \param inName the cell name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the cell id
    /// If cell ID already exists, it will overwrite its data. Otherwise, it will be appended
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or writing fails.
    /// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
    void writeCellData(isize_t inCellId, Image & inSegmentationImage, Trace<float> & inData, const std::string & inName = std::string());
    
    /// \return if the cell is valid 
    /// \param inCellId the cell of interest
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    bool isCellValid(isize_t inCellId);
    
    /// Set a cell in the set to be valid/invalid (used for rejecting or accepting segmented cell)
    /// \param inCellId the cell of interest
    /// \param inIsValid whether to reject or accept a cell in the set
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    void setCellValid(isize_t inCellId, bool inIsValid);

    /// Get the name for a cell in the set
    /// \param inCellId the cell of interest
    /// \return a string with the name 
    std::string getCellName(isize_t inCellId);

    /// Set the cell name in the cell header
    /// \param inCellId the cell of interest
    /// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
    void setCellName(isize_t inCellId, const std::string & inName);


private:

    /// True if the cell set file is valid, false otherwise.
    bool m_valid = false;
    
    /// The total number of cells in the set. 
    isize_t m_numCells = 0;

    /// The name of the cell set file.
    std::string m_fileName;

    /// The timing information of the cell set.
    TimingInfo m_timingInfo;

    /// The spacing information of the cell set.
    SpacingInfo m_spacingInfo;

    /// The header offset.
    std::ios::pos_type m_headerOffset;

    /// The cell names
    std::vector<std::string> m_cellNames;

    /// The file stream
    std::fstream m_file;

    /// The stream open mode
    std::ios_base::openmode m_openmode;

    /// Read the header to populate information about the cell set.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the header from the file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the header fails.
    void readHeader();

    /// Read the all the cell names for the cells in the datasets and hold them in memory
    ///
    /// \throw  isx::ExceptionFileIO    If reading from the file fails.
    void readCellNames();

    /// Get the name for a cell in the set
    /// \param inCellId the cell of interest
    /// \return a string with the name 
    std::string readCellName(isize_t inCellId);

    /// Write the header containing information about the cell set.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    void writeHeader();
    
    /// Seek to a specific cell in the file
    /// \param inCellId the cell ID
    /// \throw  isx::ExceptionFileIO    If seeking to a unexistent cell or reading fails.
    void seekToCell(isize_t inCellId);


    /// \return the size of the cell ID in bytes (in the cell header)
    ///
    isize_t cellIdSizeInBytes();

    /// \return the size of the valid/invalid flag in bytes (in the cell header)
    ///
    isize_t cellValiditySizeInBytes();

    /// \return the size of the cell name field in bytes (in the cell header)
    ///
    isize_t cellNameSizeInBytes();

    /// \return the size of the reserved portion of the cell header in bytes
    ///
    isize_t reservedSizeInBytes();

    /// \return the size of the segmentation image in bytes (in the cell header)
    ///
    isize_t segmentationImageSizeInBytes();

    /// \return the size of the trace in bytes (in the cell header)
    ///
    isize_t traceSizeInBytes();

    /// \return the size of the cell header in bytes (in the cell header)
    ///
    isize_t cellHeaderSizeInBytes();

    /// Flush the stream
    ///
    void flush();


};
}
#endif // ISX_CELL_SET_FILE_H
