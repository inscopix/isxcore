#ifndef ISX_CELL_SET_H
#define ISX_CELL_SET_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxCoreFwd.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxAsyncTaskResult.h"
#include "isxColor.h"

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
using GetTraceCB_t = std::function<SpFTrace_t()>;
/// The type of callback for getting a cell trace asynchronously
using CellSetGetTraceCB_t = std::function<void(AsyncTaskResult<SpFTrace_t>)>;
/// The type of callback that reads an image from disk
using GetImageCB_t = std::function<SpImage_t()>;
/// The type of callback for getting a cell image asynchronously
using CellSetGetImageCB_t = std::function<void(AsyncTaskResult<SpImage_t>)>;

/// The cell statuses 
///
enum class CellStatus {
    ACCEPTED = 0,                   ///< Cell has been reviewed and accepted
    UNDECIDED = 1,                  ///< Unreviewed or undecided
    REJECTED = 2                    ///< Rejected
};

/// \return whether this is a valid cell set object.
///
virtual
bool
isValid() const = 0;

/// Close this file for writing.  This writes the header containing
/// metadata at the end of the file.  Any attempts to write data after
/// this is called will result in an exception.
///
virtual
void
closeForWriting() = 0;

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
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent cell or writing fails.
/// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void 
writeImageAndTrace(
        isize_t inIndex,
        const SpImage_t & inImage,
        SpFTrace_t & inTrace,
        const std::string & inName = std::string()) = 0;

/// \return             The current status of the cell
/// \param  inIndex     The index of the cell.
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent cell or reading fails.
virtual 
CellStatus 
getCellStatus(isize_t inIndex) = 0;

/// \return             The current color of the cell
/// \param  inIndex     The index of the cell.
virtual
Color
getCellColor(isize_t inIndex) = 0;

/// \return             The current status of the cell
/// \param  inIndex     The index of the cell.
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent cell or reading fails.
virtual
std::string
getCellStatusString(isize_t inIndex) = 0;

/// Set a cell in the set to be valid/invalid.
///
/// This is used for rejecting or accepting a cell.
///
/// \param inIndex the cell of interest
/// \param inStatus the new status for the cell
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent cell or reading fails.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void 
setCellStatus(isize_t inIndex, CellStatus inStatus) = 0;

/// Set color of a cell in the set.
///
/// This is used for colorized view.
///
/// \param inIndex the cell of interest
/// \param inColor the new color for the cell
virtual
void
setCellColor(isize_t inIndex, const Color& inColor) = 0;

/// Get the name for a cell in the set
/// \param inIndex the cell of interest
/// \return a string with the name
virtual 
std::string 
getCellName(isize_t inIndex) = 0;

/// Set the cell name 
/// \param inIndex the cell of interest
/// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void 
setCellName(isize_t inIndex, const std::string & inName) = 0;

/// Check whether a cell is active in different portions/segments of the cell set
/// \param inIndex the cell of interest
/// \return a vector containing a boolean for each segment of a cell set
virtual 
std::vector<bool> 
getCellActivity(isize_t inIndex) const = 0;

/// Set the cell activity flags 
/// \param inIndex the cell of interest
/// \param inActive a vector with a flag for each segment of the cellset 
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void 
setCellActive(isize_t inIndex, const std::vector<bool> & inActive) = 0;

/// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
///
virtual 
void 
cancelPendingReads() = 0;

/// \return     True if this came from drawing ROIs, false otherwise.
///
virtual
bool
isRoiSet() const = 0;

/// get size of global cell set
/// \return  size of global cell set
virtual
isize_t
getSizeGlobalCS() = 0;

/// set size of global cell set
/// \param inSizeGlobalCS size of global cell set
virtual
void
setSizeGlobalCS(const isize_t inSizeGlobalCS) = 0;

/// get the cell set matches
/// \return  cell set matches
virtual
std::vector<int16_t>
getMatches() = 0;

/// set the cell set matches
/// \param inMatches cell set matches
virtual
void
setMatches(const std::vector<int16_t> & inMatches) = 0;

/// get cell set pair scores
/// \return  cell set pair scores
virtual
std::vector<double>
getPairScores() = 0;

/// set cell set pair scores
/// \param inPairScores cell set pair scores
virtual
void
setPairScores(const std::vector<double> & inPairScores) = 0;

/// get the cell set centroid distances
/// \return  cell set centroid distances
virtual
std::vector<double>
getCentroidDistances() = 0;

/// set the cell set centroid distances
/// \param inCentroidDistances cell set centroid distances
virtual
void
setCentroidDistances(const std::vector<double> & inCentroidDistances) = 0;

};

}
#endif // ISX_CELL_SET_H
