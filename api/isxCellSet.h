#ifndef ISX_CELL_SET_H
#define ISX_CELL_SET_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxCoreFwd.h"
#include "isxImage.h"
#include "isxTrace.h"
#include <string>
#include <functional>


namespace isx
{
/// Interface for cell sets
///
class CellSet
{
public:
/// The type of callback for reading a trace from disk
typedef std::function<SpFTrace_t()> GetTraceCB_t;
/// The type of callback for getting a cell trace asynchronously
typedef std::function<void(const SpFTrace_t & inTrace)> CellSetGetTraceCB_t;

/// The type of callback that reads an image from disk
typedef std::function<SpImage_t()> GetImageCB_t;
/// The type of callback for getting a cell image asynchronously
typedef std::function<void(const SpImage_t & inTrace)> CellSetGetImageCB_t;

/// \return whether this is a valid cell set object.
///
virtual
bool
isValid() const = 0;

/// \return     The name of the file storing this cell set.
///
virtual 
std::string 
getFileName() const = 0;

/// \return     The number of cells contained in this cell set.
///
virtual 
const isize_t 
getNumCells() = 0;

/// \return     The timing information read from this cell set.
///
virtual 
isx::TimingInfo 
getTimingInfo() const = 0;

/// \return     The TimingInfos of a CellSetSeries.
///             For a regular cell set this will contain one TimingInfo object
///             matching getTimingInfo.
///
virtual
isx::TimingInfos_t 
getTimingInfosForSeries() const = 0;

/// \return     The spacing information read from this cell set.
///
virtual 
isx::SpacingInfo 
getSpacingInfo() const = 0;

/// Get the trace of a cell synchronously.
///
/// This actually calls getTraceAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the cell
/// \return             A shared pointer to the trace data of the indexed cell.
/// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
virtual 
SpFTrace_t 
getTrace(isize_t inIndex) = 0;

/// Get the trace of cell asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a trace of a cell.
///
/// \param  inIndex     The index of the cell
/// \param  inCallback  The call back that operates on the trace.
virtual 
void 
getTraceAsync(isize_t inIndex, CellSetGetTraceCB_t inCallback) = 0;

/// Get the image of a cell synchronously.
///
/// This actually calls getImageAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the cell
/// \return             A shared pointer to the image data of the indexed cell.
/// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
virtual 
SpImage_t 
getImage(isize_t inIndex) = 0;

/// Get the image of cell asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a image of a cell.
///
/// \param  inIndex     The index of the cell
/// \param  inCallback  The call back that operates on the image.
virtual 
void 
getImageAsync(isize_t inIndex, CellSetGetImageCB_t inCallback) = 0;

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
virtual 
void 
writeImageAndTrace(
        isize_t inIndex,
        SpImage_t & inImage,
        SpFTrace_t & inTrace,
        const std::string & inName = std::string()) = 0;

/// \return             True if the cell is valid
/// \param  inIndex     The index of the cell.
/// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
virtual 
bool 
isCellValid(isize_t inIndex) = 0;

/// Set a cell in the set to be valid/invalid.
///
/// This is used for rejecting or accepting a cell.
///
/// \param inIndex the cell of interest
/// \param inIsValid whether to reject or accept a cell in the set
/// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
virtual 
void 
setCellValid(isize_t inIndex, bool inIsValid) = 0;

/// Get the name for a cell in the set
/// \param inIndex the cell of interest
/// \return a string with the name
virtual 
std::string 
getCellName(isize_t inIndex) = 0;

/// Set the cell name 
/// \param inIndex the cell of interest
/// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
virtual 
void 
setCellName(isize_t inIndex, const std::string & inName) = 0;

/// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
///
virtual 
void 
cancelPendingReads() = 0;

};

}
#endif // ISX_CELL_SET_H