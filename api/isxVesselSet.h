#ifndef ISX_VESSEL_SET_H
#define ISX_VESSEL_SET_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxCoreFwd.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxAsyncTaskResult.h"
#include "isxColor.h"
#include "isxMetadata.h"
#include "isxVesselCorrelations.h"

#include <json.hpp>
#include <string>
#include <functional>

namespace isx
{
/// A structure for holding the information relevant to a Vessel line, i.e. a line drawn across a blood vessel.
struct VesselLine
{
    /// Default constructor
    VesselLine() {}

/// Constructor
///
/// \param  contour     The points of the vessel line
    VesselLine(Contour_t contour) : m_contour(contour) {
        ISX_ASSERT(contour.size() == 2 || contour.size() == 4);
    }

    /// Compute the maximum limit on velocity for a given vessel line in a movie
    /// \param fps                  The frame rate of the movie
    float computeMaxVelocity(const float fps)
    {
        if (m_contour.size() != 4)
        {
            ISX_THROW(ExceptionUserInput, "Cannot compute max velocity for contour without 4 points");
        }
        if (std::isnan(fps))
        {
            return std::numeric_limits<float>::quiet_NaN();
        }

        // This algorithm assumes that adjacent points are stored next to each other in the contour
        // Determine the maximum length of the velocity bounding box edges
        float maxLength = 0.0f;
        for (size_t i = 0; i < 4; ++i)
        {
            float sideLength = getLineLength(m_contour[i], m_contour[(i + 1) % 4]);
            if (sideLength > maxLength)
            {
                maxLength = sideLength;
            }
        }
        // Calculate the max velocity by dividing the max length by 2 to account for the 3 time offsets
        // then multiply by the frame rate
        return maxLength > 0 ? (maxLength / 2.0f) * fps : std::numeric_limits<float>::quiet_NaN();
    }

    /// Compute the euclidean distance of the line between two points
    /// \param p1                   The first point defining the line
    /// \param p2                   The second point definind the line
    float getLineLength(const isx::PointInPixels_t p1, const isx::PointInPixels_t p2)
    {
        float dx = (float)(p1.getX()) - (float)(p2.getX());
        float dy = (float)(p1.getY()) - (float)(p2.getY());
        return std::sqrt(dx * dx + dy * dy);
    }

    Contour_t m_contour = Contour_t();  ///< The points of the vessel line
};

using SpVesselLine_t = std::shared_ptr<VesselLine>;

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
/// The type of callback for reading a vessel's line endpoints from disk
using GetLineEndpointsCB_t = std::function<SpVesselLine_t()>;
/// The type of callback for getting a vessel line endpoints asynchronously
using VesselSetGetLineEndpointsCB_t = std::function<void(AsyncTaskResult<SpVesselLine_t>)>;
/// The type of callback for reading a vessel's correlation from disk
using GetCorrelationsCB_t = std::function<SpVesselCorrelations_t()>;
/// The type of callback for getting a vessel's correlation asynchronously
using VesselSetGetCorrelationsCB_t = std::function<void(AsyncTaskResult<SpVesselCorrelations_t>)>;

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

/// Get the projection image synchronously.
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

/// Get the projection image asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a image of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the image.
virtual
void
getImageAsync(isize_t inIndex, VesselSetGetImageCB_t inCallback) = 0;

/// Get the line endpoints of a vessel synchronously.
///
/// This actually calls getTraceAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the vessel
/// \return             A shared pointer to the trace data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpVesselLine_t
getLineEndpoints(isize_t inIndex) = 0;

/// Get the line endpoints of a vessel asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a trace of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the trace.
virtual
void
getLineEndpointsAsync(isize_t inIndex, VesselSetGetLineEndpointsCB_t inCallback) = 0;

/// Get the direction trace of a vessel synchronously.
///
/// This actually calls getDirectionTraceAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the vessel
/// \return             A shared pointer to the direction data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpFTrace_t
getDirectionTrace(isize_t inIndex) = 0;

/// Get the direction trace of a vessel asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a direction of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the direction
virtual
void
getDirectionTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback) = 0;

/// Get the correlation triptych for a particular velocity measurement of a vessel synchronously.
///
/// This actually calls getCorrelationsAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex         The index of the vessel
/// \param  inFrameNumber   The index of the velocity measurement
/// \return             A shared pointer to the correlation triptych data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpVesselCorrelations_t
getCorrelations(isize_t inIndex, isize_t inFrameNumber) = 0;

/// Get the correlation triptych for a particular velocity measurement of a vessel from velocity set asynchronously.
///
/// This dispatches a task to the IoQueue that operates on a correlation triptychs of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inFrameNumber   The index of the velocity measurement
/// \param  inCallback  The call back that operates on the correlation triptychs
virtual
void
getCorrelationsAsync(isize_t inIndex, isize_t inFrameNumber, VesselSetGetCorrelationsCB_t inCallback) = 0;

/// Get the center trace of a vessel from a diameter set synchronously
/// Each value in the trace represents the position on the user drawn line of the model peak center for each mesurement
///
/// This actually calls getCenterTraceAsync and will wait for the asynchronous
/// task to complete.
///
/// \param  inIndex     The index of the vessel
/// \return             A shared pointer to the center trace data of the indexed vessel.
/// \throw  isx::ExceptionFileIO    If vessel does not exist or reading fails.
virtual
SpFTrace_t
getCenterTrace(isize_t inIndex) = 0;

/// Get the center trace of a vessel from a diameter set asynchronously.
/// Each value in the trace represents the pixel position of the diameter model fit peak center
/// This pixel position represents a point on the user drawn line
///
/// This dispatches a task to the IoQueue that operates on a center trace of a vessel.
///
/// \param  inIndex     The index of the vessel
/// \param  inCallback  The call back that operates on the center trace
virtual
void
getCenterTraceAsync(isize_t inIndex, VesselSetGetTraceCB_t inCallback) = 0;

/// Writes the projection image of the input movie
/// \param inProjectionImage    The projection image data to write.
/// \throw  isx::ExceptionDataIO    If image data is of an unexpected data type.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
writeImage(
    const SpImage_t & inProjectionImage) = 0;

/// Write the line endpoints, and diameter trace data for a vessel
/// If the vessel already exists, it will overwrite its data.
/// Otherwise, it will be appended.
///
/// This write is performed on the IoQueue, but this function waits
/// until it is complete.
///
/// \param  inIndex             The index of the vessel.
/// \param  inLineEndpoints     The vessel line endpoints to write.
/// \param  inDiameterTrace     The vessel diameter trace to write.
/// \param  inCenterTrace       The position on the user drawn line of the model peak center for each mesurement
/// \param  inName              The vessel name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the given index
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or writing fails.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
writeVesselDiameterData(
    const isize_t inIndex,
    const SpVesselLine_t & inLineEndpoints,
    const SpFTrace_t & inDiameterTrace,
    const SpFTrace_t & inCenterTrace,
    const std::string & inName = std::string()) = 0;

/// Write the line endpoints, and velocity trace data for a vessel
/// If the vessel already exists, it will overwrite its data.
/// Otherwise, it will be appended.
///
/// This write is performed on the IoQueue, but this function waits
/// until it is complete.
///
/// \param  inIndex             The index of the vessel.
/// \param  inLineEndpoints     The vessel line endpoints to write.
/// \param  inVelocityTrace     The vessel velocity magnitude trace to write.
/// \param  inDirectionTrace    The vessel direction trace to write.
/// \param  inCorrTrace         The correlation triptychs used to estimate velocity.
/// \param  inName              The vessel name (will be truncated to 15 characters, if longer). If no name is provided, a default will be created using the given index
/// \throw  isx::ExceptionFileIO    If trying to access nonexistent vessel or writing fails.
/// \throw  isx::ExceptionFileIO    If called after calling closeForWriting().
virtual
void
writeVesselVelocityData(
    const isize_t inIndex,
    const SpVesselLine_t & inLineEndpoints,
    const SpFTrace_t & inVelocityTrace,
    const SpFTrace_t & inDirectionTrace,
    const SpVesselCorrelationsTrace_t & inCorrTrace = nullptr,
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

/// \return     The type of the vessel set (rbc velocity or vessel diameter)
virtual
VesselSetType_t
getVesselSetType() const = 0;

/// \param  inIndex    The id of the vessel
/// \return     The size of correlation heatmaps for the vessel
virtual
SizeInPixels_t
getCorrelationSize(size_t inIndex) const = 0;

/// \param inIndex the id of the vessel
/// \return the maximum possible velocity for the vessel
virtual
float
getMaxVelocity(size_t inIndex) = 0;

/// \return     Flag indicating whether correlation heatmaps are saved to disk
virtual
bool
isCorrelationSaved() const = 0;

/// \return Flag indicating whether direction trace was saved to disk
/// This is primarily meant to maintain backwards compatibility with vessel sets generated in IDPS 1.7
///
virtual
bool
isDirectionSaved() const = 0;

/// \return Flag indicating whether center trace was saved to disk
/// This is primarily meant to maintain backwards compatibility with vessel sets generated in IDPS 1.7
///
virtual
bool
isCenterSaved() const = 0;
};

} // namespace isx

#endif // ISX_VESSEL_SET_H
