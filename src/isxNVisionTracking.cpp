#include "isxNVisionTracking.h"

#include <json.hpp>
using json = nlohmann::json;

namespace isx {

BoundingBox
BoundingBox::fromMetadata(
    const std::string & inMetadata
)
{
    const auto frameMetadata = json::parse(inMetadata);
    if (frameMetadata.is_null())
    {
        return BoundingBox();
    }

    if (frameMetadata.find("tracker") == frameMetadata.end())
    {
        return BoundingBox();
    }
    const auto trackerMetadata = frameMetadata.at("tracker");

    if (trackerMetadata.find("box") == trackerMetadata.end())
    {
        return BoundingBox();
    }
    const auto trackerBoxMetadata = trackerMetadata.at("box");

    int64_t zoneId = -1;
    if (trackerMetadata.find("zones") != trackerMetadata.end())
    {
        const auto zoneMetadata = trackerMetadata.at("zones");

        if (zoneMetadata.find("id") != zoneMetadata.end())
        {
            zoneId = zoneMetadata.at("id").get<int64_t>();
        }
    }

    return BoundingBox(
        trackerBoxMetadata[1].get<float>(),
        trackerBoxMetadata[0].get<float>(),
        trackerBoxMetadata[3].get<float>(),
        trackerBoxMetadata[2].get<float>(),
        trackerMetadata.at("conf").get<float>(),
        zoneId
    );
}

bool
BoundingBox::isValid() const
{
    return (m_top || m_bottom) && (m_left || m_right);
}

float
BoundingBox::getTop() const
{
    return m_top;
}

float
BoundingBox::getLeft() const
{
    return m_left;
}

float
BoundingBox::getBottom() const
{
    return m_bottom;
}

float
BoundingBox::getRight() const
{
    return m_right;
}

SpatialPoint<float>
BoundingBox::getCenter() const
{
    return SpatialPoint<float>(
        (m_left + m_right) / 2.0f,
        (m_top + m_bottom) / 2.0f
    );
}

float
BoundingBox::getConfidence() const
{
    return m_confidence;
}

int64_t
BoundingBox::getZoneId() const
{
    return m_zoneId;
}

int64_t 
Zone::getId() const
{
    return m_id;
}

bool
Zone::getEnabled() const
{
    return m_enabled;
}

std::string
Zone::getName() const
{
    return m_name;
}

std::string
Zone::getDescription() const
{
    return m_description;
}

Zone::Type
Zone::getType() const
{
    return m_type;
}

std::vector<SpatialPoint<float>>
Zone::getCoordinates() const
{
    return m_coordinates;
}

float
Zone::getMajorAxis() const
{
    return m_majorAxis;
}

float 
Zone::getMinorAxis() const
{
    return m_minorAxis;
}

float
Zone::getAngle() const
{
    return m_angle;
}

Zone::Type
Zone::getTypeFromString(
    const std::string inStr
)
{
    if (inStr == "rectangle")
    {
        return Zone::Type::RECTANGLE;
    }

    if (inStr == "polygon")
    {
        return Zone::Type::POLYGON;
    }

    if (inStr == "ellipse")
    {
        return Zone::Type::ELLIPSE;
    }

    return Zone::Type::INVALID;
}

std::string
Zone::getZoneTypeString() const
{
    if (m_type == Zone::Type::RECTANGLE)
    {
        return "rectangle";
    }

    if (m_type == Zone::Type::POLYGON)
    {
        return "polygon";
    }

    if (m_type == Zone::Type::ELLIPSE)
    {
        return "ellipse";
    }

    return "";
}

std::vector<Zone>
getZonesFromMetadata(
    const std::string & inMetadata
)
{
    std::vector<Zone> zones;
    const auto extraProps = json::parse(inMetadata);
    if (
        extraProps.find("trackingInterface") == extraProps.end() ||
        extraProps.find("processingInterface") == extraProps.end()
    )
    {
        ISX_LOG_WARNING("No trackingInterface or processingInterface in isxb session metadata, cannot retrieve zones.");
        return zones;
    }


    std::string cameraName = extraProps.at("cameraName").get<std::string>();
    const auto processingInterface = extraProps.at("processingInterface");
    if (processingInterface.find(cameraName) != processingInterface.end())
    {
        const auto cameraSection = processingInterface.at(cameraName);
        if (cameraSection.find("cameraAlias") != cameraSection.end())
        {
            const auto cameraAlias = cameraSection.at("cameraAlias").get<std::string>();
            if (!cameraAlias.empty())
            {
                cameraName = cameraAlias;
            }
        }
    }

    json zonesMetadata;
    const auto trackingInterface = extraProps.at("trackingInterface");
    for (auto cameraSection : trackingInterface)
    {
        if (cameraSection.at("cameraName").get<std::string>() == cameraName)
        {
            zonesMetadata = cameraSection.at("zones");
            break;
        }
    }

    if (zonesMetadata.is_null())
    {
        ISX_LOG_WARNING("Could not extract zones from isxb session metadata");
        return zones;
    }

    for (auto zoneMetadata : zonesMetadata)
    {
        const int64_t id = zoneMetadata.at("id").get<int64_t>();
        const bool enabled = zoneMetadata.at("enable").get<bool>();
        const std::string name = zoneMetadata.at("name").get<std::string>();
        const std::string description = zoneMetadata.at("description").get<std::string>();

        const auto zoneGeometry = zoneMetadata.at("geometry");
        const Zone::Type type = Zone::getTypeFromString(zoneGeometry.at("type").get<std::string>());

        std::vector<SpatialPoint<float>> coordinates;
        const auto zoneCoordinates = zoneGeometry.at("coordinates");
        for (auto coordinate : zoneCoordinates)
        {
            coordinates.push_back(
                SpatialPoint<float>(
                    coordinate[0].get<float>(),
                    coordinate[1].get<float>()
                )
            );
        }
        
        float majorAxis = 0.0f, minorAxis = 0.0f, angle = 0.0f;
        if (zoneGeometry.find("properties") != zoneGeometry.end())
        {
            const auto zoneGeometryProps = zoneGeometry.at("properties");

            if (zoneGeometryProps.find("majorAxis") != zoneGeometryProps.end())
            {
                majorAxis = zoneGeometryProps.at("majorAxis").get<float>();
            }

            if (zoneGeometryProps.find("minorAxis") != zoneGeometryProps.end())
            {
                minorAxis = zoneGeometryProps.at("minorAxis").get<float>();
            }

            if (zoneGeometryProps.find("angle") != zoneGeometryProps.end())
            {
                angle = zoneGeometryProps.at("angle").get<float>();
            }

        }

        zones.push_back(
            Zone(
                id,
                enabled,
                name,
                description,
                type,
                coordinates,
                majorAxis,
                minorAxis,
                angle
            )
        );
    }

    return zones;
}

} // namespace isx
