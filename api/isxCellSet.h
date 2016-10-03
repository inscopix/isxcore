#ifndef ISX_CELL_SET_H
#define ISX_CELL_SET_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// Forward declare needed because CellSetFile is in src.
class CellSetFile;

/// A class for a containing cells extracted from a movie
///
/// Currently all reads/writes happen on calling threads and
/// not on the IO thread.
class CellSet : public std::enable_shared_from_this<CellSet>
{
public:

    /// The type of callback for getting a cell trace asynchronously
    typedef std::function<void(const SpFTrace_t & inTrace)> GetTraceCB_t;

    /// The type of callback for getting a cell image asynchronously
    typedef std::function<void(const SpImage_t & inTrace)> GetImageCB_t;

    /// Empty constructor.
    ///
    /// This creates an invalid cell set and is for allocation purposes only.
    CellSet();

    /// Read constructor.
    ///
    /// This opens an existing cell set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the cell set file to read.
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSet(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the cell set file to write.
    /// \param  inTimingInfo    The timing information of the cell set.
    /// \param  inSpacingInfo   The spacing information of the cell set.
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    CellSet(const std::string & inFileName,
            const TimingInfo & inTimingInfo,
            const SpacingInfo & inSpacingInfo);

    /// Destructor.
    ///
    ~CellSet();

    /// \return True if this cell set is valid, false otherwise.
    ///
    bool isValid() const;

    /// \return     The name of the file storing this cell set.
    ///
    std::string getFileName() const;

    /// \return     The number of cells contained in this cell set.
    ///
    const isize_t getNumCells();

    /// \return     The timing information read from this cell set.
    ///
    isx::TimingInfo getTimingInfo() const;

    /// \return     The spacing information read from this cell set.
    ///
    isx::SpacingInfo getSpacingInfo() const;

    /// Get the trace of a cell synchronously.
    ///
    /// This actually calls getTraceAsync and will wait for the asynchronous
    /// task to complete.
    ///
    /// \param  inIndex     The index of the cell
    /// \return             A shared pointer to the trace data of the indexed cell.
    /// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
    SpFTrace_t getTrace(isize_t inIndex);

    /// Get the trace of cell asynchronously.
    ///
    /// This dispatches a task to the IoQueue that operates on a trace of a cell.
    ///
    /// \param  inIndex     The index of the cell
    /// \param  inCallback  The call back that operates on the trace.
    void getTraceAsync(isize_t inIndex, GetTraceCB_t inCallback);

    /// Get the image of a cell synchronously.
    ///
    /// This actually calls getImageAsync and will wait for the asynchronous
    /// task to complete.
    ///
    /// \param  inIndex     The index of the cell
    /// \return             A shared pointer to the image data of the indexed cell.
    /// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
    SpImage_t getImage(isize_t inIndex);

    /// Get the image of cell asynchronously.
    ///
    /// This dispatches a task to the IoQueue that operates on a image of a cell.
    ///
    /// \param  inIndex     The index of the cell
    /// \param  inCallback  The call back that operates on the image.
    void getImageAsync(isize_t inIndex, GetImageCB_t inCallback);

    /// Write the image and trace data for a cell.
    ///
    /// If the cell already exists, it will overwrite its data.
    /// Otherwise, it will be appended.
    ///
    /// This write is performed on the IoQueue, but this function waits
    /// until it is complete.
    ///
    /// \param  inIndex     The index of the cell.
    /// \param  inImage     The cell image data to write.
    /// \param  inTrace     The cell trace data to write.
    /// \param  inName      The cell name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the given index
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or writing fails.
    /// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
    void writeImageAndTrace(
            isize_t inIndex,
            SpImage_t & inImage,
            SpFTrace_t & inTrace,
            const std::string & inName = std::string());

    /// \return             True if the cell is valid
    /// \param  inIndex     The index of the cell.
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    bool isCellValid(isize_t inIndex);

    /// Set a cell in the set to be valid/invalid.
    ///
    /// This is used for rejecting or accepting a cell.
    ///
    /// \param inIndex the cell of interest
    /// \param inIsValid whether to reject or accept a cell in the set
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    void setCellValid(isize_t inIndex, bool inIsValid);

    /// Get the name for a cell in the set
    /// \param inIndex the cell of interest
    /// \return a string with the name
    std::string 
    CellSet::getCellName(isize_t inIndex);

    /// Set the cell name 
    /// \param inIndex the cell of interest
    /// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
    void 
    CellSet::setCellName(isize_t inIndex, const std::string & inName);

    /// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
    ///
    void cancelPendingReads();

private:

    /// True if the cell set is valid, false otherwise.
    bool m_valid = false;

    /// The cell set file to read/write from/to.
    std::shared_ptr<CellSetFile> m_file;

    /// The number of read requests since this was created.
    uint64_t m_readRequestCount = 0;

    /// The mutex to control write access to the map of pending read requests.
    isx::Mutex m_pendingReadsMutex;

    /// The pending read requests.
    std::map<uint64_t, SpAsyncTaskHandle_t> m_pendingReads;

    /// Remove read request from our pending reads.
    //
    /// \param  inReadRequestId Id of request to remove.
    /// \return                 AsyncTaskHandle for the removed read request.
    SpAsyncTaskHandle_t unregisterReadRequest(uint64_t inReadRequestId);

};

}
#endif // ISX_CELL_SET_H
