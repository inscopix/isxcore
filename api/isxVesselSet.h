#ifndef ISX_VESSEL_SET_H
#define ISX_VESSEL_SET_H

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
/// Interface for vessel sets
///
class VesselSet
{
public:
/// The type of callback for reading a trace from disk
using GetTraceCB_t = std::function<SpFTrace_t()>;
/// The type of callback for getting a vessel diameter trace asynchronously
using VesselSetGetTraceCB_t = std::function<void(AsyncTaskResult<SpFTrace_t>)>;
/// The type of callback that reads an image from disk
using GetImageCB_t = std::function<SpImage_t()>;
/// The type of callback for getting a vessel image asynchronously
using VesselSetGetImageCB_t = std::function<void(AsyncTaskResult<SpImage_t>)>;

/// The vessel statuses
///
enum class VesselStatus {
    ACCEPTED = 0,                   ///< Vessel has been reviewed and accepted
    UNDECIDED = 1,                  ///< Unreviewed or undecided
    REJECTED = 2                    ///< Rejected
};

/// \return whether this is a valid vessel set object.
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

/// \return     The name of the file storing this vessel set.
///
virtual
std::string
getFileName() const = 0;

/// \return     The number of vessels contained in this vessel set.
///
virtual
const isize_t
getNumVessels() = 0;

/// \return     The timing information read from this vessel set.
///
virtual
isx::TimingInfo
getTimingInfo() const = 0;

/// \return     The TimingInfos of a VesselSetSeries.
///             For a regular vessel set this will contain one TimingInfo object
///             matching getTimingInfo.
///
virtual
isx::TimingInfos_t
getTimingInfosForSeries() const = 0;

/// \return     The spacing information read from this vessel set.
///
virtual
isx::SpacingInfo
getSpacingInfo() const = 0;

/// Get the trace of a vessel synchronously.
///
/// This actually calls getTraceAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the vessel
/// \return             A shared pointer to the trace data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpFTrace_t
getTrace(isize_t inIndex) = 0;

/// Get the trace of vessel asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a trace of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the trace.
virtual
void
getTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback) = 0;

/// Get the image of a vessel synchronously.
///
/// This actually calls getImageAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the vessel
/// \return             A shared pointer to the image data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpImage_t
getImage(isize_t inIndex) = 0;

/// Get the image of vessel asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a image of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the image.
virtual
void
getImageAsync(isize_t inIndex, VesselSetGetImageCB_t inCallback) = 0;

/// Write the image and trace data for a vessel.
///
/// If the vessel already exists, it will overwrite its data.
/// Otherwise, it will be appended.
///
/// This write is performed on the IoQueue, but this function waits
/// until it is complete.
///
/// \param  inIndex     The index of the vessel.
/// \param  inImage     The vessel image data to write.
/// \param  inTrace     The vessel trace data to write.
/// \param  inName      The vessel name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the given index
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or writing fails.
/// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
writeImageAndTrace(
        isize_t inIndex,
        const SpImage_t & inImage,
        SpFTrace_t & inTrace,
        const std::string & inName = std::string()) = 0;

/// \return             The current status of the vessel
/// \param  inIndex     The index of the vessel.
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
virtual
VesselStatus
getVesselStatus(isize_t inIndex) = 0;

/// \return             The current color of the vessel
/// \param  inIndex     The index of the vessel.
virtual
Color
getVesselColor(isize_t inIndex) = 0;

/// \return             The current status of the vessel
/// \param  inIndex     The index of the vessel.
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
virtual
std::string
getVesselStatusString(isize_t inIndex) = 0;

/// Set a vessel in the set to be valid/invalid.
///
/// This is used for rejecting or accepting a vessel.
///
/// \param inIndex the vessel of interest
/// \param inStatus the new status for the vessel
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or reading fails.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
setVesselStatus(isize_t inIndex, VesselStatus inStatus) = 0;

/// Set color of a vessel in the set.
///
/// This is used for colorized view.
///
/// \param inIndex the vessel of interest
/// \param inColor the new color for the vessel
virtual
void
setVesselColor(isize_t inIndex, const Color& inColor) = 0;

/// Set colors of a vessels in the set.
///
/// This is used for colorized view.
///
/// \param inColors IdColorPairs
virtual
void
setVesselColors(const IdColorPairs &inColors) = 0;

/// Get the name for a vessel in the set
/// \param inIndex the vessel of interest
/// \return a string with the name
virtual
std::string
getVesselName(isize_t inIndex) = 0;

/// Set the vessel name
/// \param inIndex the vessel of interest
/// \param inName the assigned name (it will be truncated to 15 characters, if longer than that)
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
setVesselName(isize_t inIndex, const std::string & inName) = 0;

/// Check whether a vessel is active in different portions/segments of the vessel set
/// \param inIndex the vessel of interest
/// \return a vector containing a boolean for each segment of a vessel set
virtual
std::vector<bool>
getVesselActivity(isize_t inIndex) const = 0;

/// Set the vessel activity flags
/// \param inIndex the vessel of interest
/// \param inActive a vector with a flag for each segment of the vessel set
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
setVesselActive(isize_t inIndex, const std::vector<bool> & inActive) = 0;

/// Cancel all pending read requests (schedule with getTraceAsync/getImageAsync).
///
virtual
void
cancelPendingReads() = 0;

/// get the efocus values
/// \return  efocus values
virtual
std::vector<uint16_t>
getEfocusValues() = 0;

/// set the efocus values
/// \param inEfocus efocus value
virtual
void
setEfocusValues(const std::vector<uint16_t> & inEfocus) = 0;

/// \return     The extra properties of this which might include things
///             from nVista 3. The string is in JSON format.
virtual
std::string
getExtraProperties() const = 0;

/// \param  inProperties    The extra properties formatted as a JSON string.
///
virtual
void
setExtraProperties(const std::string & inProperties) = 0;

/// \return     The original spacing info of this vessel set on the sensor the corresponding
///             movie was captured with. Prior to nVista 3, the assumption is that all
///             microscope movies use the 1440x1080 sensor with 2.2x2.2 micron size.
virtual
SpacingInfo
getOriginalSpacingInfo() const = 0;

};

} // namespace isx

#endif // ISX_VESSEL_SET_H
