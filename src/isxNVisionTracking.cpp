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

    const auto trackerMetadata = frameMetadata.find("tracker");
    if (trackerMetadata == frameMetadata.end())
    {
        return BoundingBox();
    }

    const auto trackerBoxMetadata = trackerMetadata->find("box");
    if (trackerBoxMetadata == trackerMetadata->end())
    {
        return BoundingBox();
    }

    int64_t zoneId = -1;
    const auto zoneMetadata = trackerMetadata->find("zones");
    if (zoneMetadata != trackerMetadata->end())
    {
        zoneId = zoneMetadata->at("id").get<int64_t>();
    }

    return BoundingBox(
        (*trackerBoxMetadata)[1].get<float>(),
        (*trackerBoxMetadata)[0].get<float>(),
        (*trackerBoxMetadata)[3].get<float>(),
        (*trackerBoxMetadata)[2].get<float>(),
        trackerMetadata->at("conf").get<float>(),
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
    if (extraProps.find("trackingInterface") == extraProps.end())
    {
        return zones;
    }

    const auto cameraName = extraProps.at("cameraName").get<std::string>();
    json zonesMetadata;
    for (auto cameraSection : extraProps.at("trackingInterface"))
    {
        if (cameraSection.at("cameraName").get<std::string>() == cameraName)
        {
            zonesMetadata = cameraSection.at("zones");
            break;
        }
    }

    for (auto zoneMetadata : zonesMetadata)
    {
        const int64_t id = zoneMetadata["id"].get<int64_t>();
        const bool enabled = zoneMetadata["enable"].get<bool>();
        const std::string name = zoneMetadata["name"].get<std::string>();
        const std::string description = zoneMetadata["description"].get<std::string>();
        const Zone::Type type = Zone::getTypeFromString(zoneMetadata["geometry"]["type"].get<std::string>());

        std::vector<SpatialPoint<float>> coordinates;
        for (auto coordinate : zoneMetadata["geometry"]["coordinates"])
        {
            coordinates.push_back(
                SpatialPoint<float>(
                    coordinate[0].get<float>(),
                    coordinate[1].get<float>()
                )
            );
        }

        const float majorAxis = (
            zoneMetadata["geometry"]["properties"]["majorAxis"].is_null() ?
            0 : zoneMetadata["geometry"]["properties"]["majorAxis"].get<float>()
        );

        const float minorAxis = (
            zoneMetadata["geometry"]["properties"]["minorAxis"].is_null() ?
            0 : zoneMetadata["geometry"]["properties"]["minorAxis"].get<float>()
        );

        const float angle = (
            zoneMetadata["geometry"]["properties"]["angle"].is_null() ?
            0 : zoneMetadata["geometry"]["properties"]["angle"].get<float>()
        );

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
