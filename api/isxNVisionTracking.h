#ifndef ISX_NVISION_TRACKING_H
#define ISX_NVISION_TRACKING_H

#include "isxCore.h"
#include "isxSpacingInfo.h"
#include "isxCoreFwd.h"

namespace isx
{

/// Bounding box information tracked by the nVision system.
class BoundingBox {
    public:
        /// Default constructor.
        BoundingBox()
        {}

        /// Constructor.
        ///
        /// \param inTop    The top pixel coordinate of the bounding box.
        /// \param inLeft    The left pixel coordinate of the bounding box.
        /// \param inBottom    The bottom pixel coordinate of the bounding box.
        /// \param inRight    The right pixel coordinate of the bounding box.
        /// \param inConfidence    The confidence of the model.
        /// \param inZoneId    The zone id the bounding box occupies.
        /// \param inZoneName   The zone name.
        BoundingBox(
            const float inTop,
            const float inLeft,
            const float inBottom,
            const float inRight,
            const float inConfidence,
            const int64_t inZoneId,
            const std::string inZoneName
        )
        : m_top(inTop)
        , m_left(inLeft)
        , m_bottom(inBottom)
        , m_right(inRight)
        , m_confidence(inConfidence)
        , m_zoneId(inZoneId)
        , m_zoneName(inZoneName)
        {
        }

        /// Create a bounding box from nVision movie frame metadata.
        ///
        /// \param inMetadata   The frame metadata from the nVision system.
        ///                     Input is formatted as a json serialized string.
        static
        BoundingBox
        fromMetadata(
            const std::string & inMetadata
        );

        /// Flag indicating whether the bounding box is valid.
        /// If false, then there is no nVision tracking data for that frame.
        bool
        isValid() const;

        /// Get the top pixel coordinate of the bounding box.
        float
        getTop() const;

        /// Get the left pixel coordinate of the bounding box.
        float
        getLeft() const;

        /// Get the bottom pixel coordinate of the bounding box.
        float
        getBottom() const;

        /// Get the right pixel coordinate of the bounding box.
        float
        getRight() const;

        /// Calculates the center point of the bounding box.
        SpatialPoint<float>
        getCenter() const;

        /// Get the model confidence.
        float
        getConfidence() const;

        /// Get the zone id that the bounding box occupies.
        /// Returns -1 if there is no zone identified.
        int64_t
        getZoneId() const;

        /// Get the zone name
        std::string
        getZoneName() const;

    private:
        /// The top pixel coordinate of the bounding box.
        float m_top = 0;

        /// The left pixel coordinate of the bounding box.
        float m_left = 0;

        /// The bottom pixel coordinate of the bounding box.
        float m_bottom = 0;

        /// The right pixel coordinate of the bounding box.
        float m_right = 0;

        /// The model confidence.
        float m_confidence = 0;

        /// The zone id that the bounding box occupies.
        /// Equals -1 if there is no zone identified.
        int64_t m_zoneId = 0;

        /// The zone name
        std::string m_zoneName = "";

}; //class


/// Equality operator for BoundingBox object.
/// \param lhs  The bounding box on the left hand size of the equality operation.
/// \param rhs  The bounding box on the right hand size of the equality operation.
inline
bool operator==(
    const BoundingBox & lhs,
    const BoundingBox & rhs
) {
    return (
        lhs.getTop() == rhs.getTop() &&
        lhs.getLeft() == rhs.getLeft() &&
        lhs.getBottom() == rhs.getBottom() &&
        lhs.getRight() == rhs.getRight() &&
        lhs.getConfidence() == rhs.getConfidence() &&
        lhs.getZoneId() == rhs.getZoneId() &&
        lhs.getZoneName() == rhs.getZoneName()
    );
}

/// Zone object information used by the nVision system for tracking.
class Zone
{
    public:
        /// The zone shape type.
        enum Type
        {
            INVALID = 0,
            RECTANGLE,
            POLYGON,
            ELLIPSE
        };

        /// Default constructor.
        Zone()
        {}

        /// Constructor.
        ///
        /// \param inId     Unique identifier for the zone.
        /// \param inEnabled    Flag indicating whether zone is enabled for tracking.
        /// \param inName   User friendly name of the zone.
        /// \param inDescription    Optional description for the zone.
        /// \param inType   The zone shape type.
        /// \param inCoordinates    List of coordinates defining the shape of the zone.
        /// \param inMajorAxis  Only used for ellipse shaped zones. Length of the major axis.
        /// \param inMinorAxis  Only used for ellipse shaped zones. Length of the minor axis.
        /// \param inAngle  Only used for ellipse shaped zones. Ellipse rotation angle in degrees.
        Zone(
            const int64_t inId,
            const bool inEnabled,
            const std::string inName,
            const std::string inDescription,
            const Type inType,
            const std::vector<SpatialPoint<float>> inCoordinates,
            const float inMajorAxis = 0.0f,
            const float inMinorAxis = 0.0f,
            const float inAngle = 0.0f
        )
        : m_id(inId)
        , m_enabled(inEnabled)
        , m_name(inName)
        , m_description(inDescription)
        , m_type (inType)
        , m_coordinates(inCoordinates)
        , m_majorAxis(inMajorAxis)
        , m_minorAxis(inMinorAxis)
        , m_angle(inAngle)
        {}

        /// Get the unique identifier for the zone.
        int64_t getId() const;

        /// Get a flag indicating whether zone is enabled for tracking.
        bool getEnabled() const;

        /// Get the user friendly name of the zone.
        std::string getName() const;

        /// Get an optional description for the zone.
        std::string getDescription() const;

        /// Get the zone shape type.
        Type getType() const;

        /// Get a list of coordinates defining the shape of the zone.
        std::vector<SpatialPoint<float>>
        getCoordinates() const;

        /// Get the length of the major axis.
        /// Only used for ellipse shaped zones.
        float getMajorAxis() const;

        /// Get the length of the minor axis.
        /// Only used for ellipse shaped zones.
        float getMinorAxis() const;

        /// Get the ellipse rotation angle in degrees.
        /// Only used for ellipse shaped zones.
        float getAngle() const;

        /// Get the zone type based on a string from metadata
        static
        Type
        getTypeFromString(
            const std::string inStr
        );

        /// Get the zone type string label
        std::string
        getZoneTypeString() const;

    private:
        /// Unique identifier for the zone.
        int64_t m_id; 

        /// Flag indicating whether zone is enabled for tracking.
        bool m_enabled;

        /// User friendly name of the zone.
        std::string m_name;

        /// Optional description for the zone.
        std::string m_description;

        /// The zone shape type.
        Type m_type;

        /// List of coordinates defining the shape of the zone.
        std::vector<SpatialPoint<float>> m_coordinates;

        /// Only used for ellipse shaped zones.
        /// Length of the major axis.
        float m_majorAxis;

        /// Only used for ellipse shaped zones.
        /// Length of the minor axis.
        float m_minorAxis;

        /// Only used for ellipse shaped zones.
        /// Ellipse rotation angle in degrees.
        float m_angle;
};


/// Equality operator for Zone object
/// \param lhs  The zone on the left hand size of the equality operation.
/// \param rhs  The zone on the right hand size of the equality operation.
inline
bool operator==(
    const Zone & lhs,
    const Zone & rhs
) {
    const auto lhsCoordinates = lhs.getCoordinates();
    const auto rhsCoordinates = rhs.getCoordinates();
    if (lhsCoordinates.size() != rhsCoordinates.size())
    {
        return false;
    }

    const auto numCoordinates = lhsCoordinates.size();
    for (size_t i = 0; i < numCoordinates; i++)
    {
        if (lhsCoordinates[i].getX() != rhsCoordinates[i].getX()
            || lhsCoordinates[i].getY() != rhsCoordinates[i].getY())
        {
            return false;
        }
    }

    return (
        lhs.getId() == rhs.getId() &&
        lhs.getEnabled() == rhs.getEnabled() &&
        lhs.getName() == rhs.getName() &&
        lhs.getDescription() == rhs.getDescription() &&
        lhs.getType() == rhs.getType() &&
        lhs.getMajorAxis() == rhs.getMajorAxis() &&
        lhs.getMinorAxis() == rhs.getMinorAxis() &&
        lhs.getAngle() == rhs.getAngle()
    );
}


/// Get vector of zone objects from metadata.
/// \param inMetadata The nVision session metadata.
/// Stored in the extra properties of the nVision movie class
std::vector<Zone>
getZonesFromMetadata(
    const std::string & inMetadata
);

} // namespace

#endif // ISX_NVISION_TRACKING_H
