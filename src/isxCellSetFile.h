#ifndef ISX_CELL_SET_FILE_H
#define ISX_CELL_SET_FILE_H

#include <fstream>
#include "isxCoreFwd.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxJsonUtils.h"
#include "isxCellSet.h" 


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
    /// \param  enableWrite     Set to true to open in read-write mode
    ///
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSetFile(const std::string & inFileName, bool enableWrite = false);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the cell set file.
    /// \param  inTimingInfo    The timing information of the cell set.
    /// \param  inSpacingInfo   The spacing information of the cell set.
    /// \param  inIsRoiSet      True if this came from drawing ROIs, false otherwise.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    CellSetFile(const std::string & inFileName,
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo,
                const bool inIsRoiSet = false);

    /// Destructor.
    ///
    ~CellSetFile();
    
    /// \return True if the cell set file is valid, false otherwise.
    ///
    bool isValid() const; 

    /// Close this file for writing.  This writes the header containing
    /// metadata at the end of the file.  Any attempts to write data after
    /// this is called will result in an exception.
    ///
    void
    closeForWriting();
    
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
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void writeCellData(isize_t inCellId, const Image & inSegmentationImage, Trace<float> & inData, const std::string & inName = std::string());
    
    /// \return the status of the cell
    /// \param inCellId the cell of interest
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    CellSet::CellStatus getCellStatus(isize_t inCellId);

    /// \return the status of the cell
    /// \param inCellId the cell of interest
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    Color getCellColor(isize_t inCellId);


    /// \return the status of the cell
    /// \param inCellId the cell of interest
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    std::string
    getCellStatusString(isize_t inCellId);
    
    /// Set a cell in the set to be valid/invalid (used for rejecting or accepting segmented cell)
    /// \param inCellId the cell of interest
    /// \param inStatus accepted/rejected/undecided status
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void setCellStatus(isize_t inCellId, CellSet::CellStatus inStatus);

    /// Set a cell color
    /// \param inCellId the cell of interest
    /// \param inColor new color
    void setCellColor(isize_t inCellId, const Color& inColor);

    /// Get the name for a cell in the set
    /// \param inCellId the cell of interest
    /// \return a string with the name 
    std::string getCellName(isize_t inCellId);

    /// Set the cell name in the cell header
    /// \param inCellId the cell of interest
    /// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
    /// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
    void setCellName(isize_t inCellId, const std::string & inName);

    /// Indicates whether a cell is active in the file or not
    /// \return true if cell has a valid trace 
    bool isCellActive(isize_t inCellId) const;

    /// Set the cell activity flag
    /// \param inCellId the cell of interest
    /// \param inActive whether the cell is active or not
    void setCellActive(isize_t inCellId, bool inActive);

    /// \return     True if this came from drawing ROIs, false otherwise.
    ///
    bool isRoiSet() const;

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
    CellNames_t m_cellNames;
    
    /// Cell validity flags
    CellStatuses_t m_cellStatuses;

    /// Cell validity flags
    CellColors_t m_cellColors;

    /// Flag indicating whether a cell is active in this file
    CellActivities_t m_cellActivity;

    /// The file stream
    std::fstream m_file;

    /// The stream open mode
    std::ios_base::openmode m_openmode;
    
    bool m_fileClosedForWriting = false;

    const static size_t s_version = 2;

    /// True if this came from drawing ROIs, false otherwise.
    bool m_isRoiSet = false;

    /// Read the header to populate information about the cell set.
    ///
    /// \throw  isx::ExceptionFileIO    If reading the header from the file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the header fails.
    void readHeader();

    /// Write the header containing information about the cell set.
    ///
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    void writeHeader();
    
    /// Seek to a specific cell in the file
    /// \param inCellId the cell ID
    /// \throw  isx::ExceptionFileIO    If seeking to a unexistent cell or reading fails.
    void seekToCell(isize_t inCellId);

    /// \return the size of the segmentation image in bytes (in the cell header)
    ///
    isize_t segmentationImageSizeInBytes();

    /// \return the size of the trace in bytes (in the cell header)
    ///
    isize_t traceSizeInBytes();

    /// Flush the stream
    ///
    void flush();

    /// Replace empty cell names with names of format C%0Xu.
    /// X is the width of the number string and is automatically determined by
    /// the number of cells.
    void replaceEmptyNames();

};
}
#endif // ISX_CELL_SET_FILE_H
