#include "isxNVisionTracking.h"

#include <json.hpp>
using json = nlohmann::json;

namespace isx {

const std::map<ZoneEvent::Type, std::string> ZoneEvent::s_typeToStrMap = {
    {ZoneEvent::Type::NONE, ""},
    {ZoneEvent::Type::ENTRY, "e"},
    {ZoneEvent::Type::OCCUPIED, "o"},
    {ZoneEvent::Type::EXIT, "x"}
};

const std::map<ZoneEvent::Type, std::string> ZoneEvent::s_typeToFullStrMap = {
    {ZoneEvent::Type::NONE, ""},
    {ZoneEvent::Type::ENTRY, "entry"},
    {ZoneEvent::Type::OCCUPIED, "occupying"},
    {ZoneEvent::Type::EXIT, "exit"}
};


const std::map<ZoneEvent::Trigger, std::string> ZoneEvent::s_triggerToStrMap = {
    {ZoneEvent::Trigger::NONE, ""},
    {ZoneEvent::Trigger::SOFT_TRIG_1, "softTrig-1"},
    {ZoneEvent::Trigger::SOFT_TRIG_2, "softTrig-2"},
    {ZoneEvent::Trigger::SOFT_TRIG_3, "softTrig-3"},
    {ZoneEvent::Trigger::SOFT_TRIG_4, "softTrig-4"}
};

ZoneEvent::Type
ZoneEvent::strToType(const std::string str)
{
    for (const auto & entry : s_typeToStrMap)
    {
        if (entry.second == str)
        {
            return entry.first;
        }
    }

    ISX_THROW(ExceptionFileIO, "Failed to recognize zone event from str: ", str);
}

std::string
ZoneEvent::typeToStr(const ZoneEvent::Type type)
{
    return s_typeToStrMap.at(type);
}

std::string
ZoneEvent::typeToFullStr(const ZoneEvent::Type type)
{
    return s_typeToFullStrMap.at(type);
}

ZoneEvent::Trigger
ZoneEvent::strToTrigger(const std::string str)
{
    for (const auto & entry : s_triggerToStrMap)
    {
        if (entry.second == str)
        {
            return entry.first;
        }
    }

    ISX_THROW(ExceptionFileIO, "Failed to recognize zone trigger from str: ", str);
}

std::string
ZoneEvent::triggerToStr(const ZoneEvent::Trigger trigger)
{
    return s_triggerToStrMap.at(trigger);
}

ZoneEvent
ZoneEvent::fromMetadata(
    const std::string & inMetadata
)
{
    const auto zoneMetadata = json::parse(inMetadata);

    const auto zoneId = zoneMetadata.at("id").get<int64_t>();
    const auto zoneName = zoneMetadata.at("name").get<std::string>();

    auto type = Type::NONE;
    if (zoneMetadata.find("event") != zoneMetadata.end())
    {
        type = strToType(zoneMetadata.at("event").get<std::string>());
    }

    auto trigger = Trigger::NONE;
    if (zoneMetadata.find("trig") != zoneMetadata.end())
    {
        trigger = strToTrigger(zoneMetadata.at("trig").get<std::string>());
    }
    
    return ZoneEvent(
        zoneId,
        zoneName,
        type,
        trigger
    );
}

int64_t
ZoneEvent::getZoneId() const
{
    return m_zoneId;
}

std::string
ZoneEvent::getZoneName() const
{
    return m_zoneName;
}

ZoneEvent::Type
ZoneEvent::getType() const
{
    return m_type;
}

ZoneEvent::Trigger
ZoneEvent::getTrigger() const
{
    return m_trigger;
}


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

    std::vector<ZoneEvent> zoneEvents = {};
    if (trackerMetadata.find("zones") != trackerMetadata.end())
    {
        const auto zoneMetadata = trackerMetadata.at("zones");
        if (zoneMetadata.is_array())
        {
            for (const auto & zone : zoneMetadata)
            {
                zoneEvents.push_back(
                    ZoneEvent::fromMetadata(zone.dump())
                );
            }
        }
        else
        {
            zoneEvents.push_back(
                ZoneEvent::fromMetadata(zoneMetadata.dump())
            );
        }

    }

    return BoundingBox(
        trackerBoxMetadata[1].get<float>(),
        trackerBoxMetadata[0].get<float>(),
        trackerBoxMetadata[3].get<float>(),
        trackerBoxMetadata[2].get<float>(),
        trackerMetadata.at("conf").get<float>(),
        zoneEvents
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

std::vector<ZoneEvent>
BoundingBox::getZoneEvents() const
{
    return m_zoneEvents;
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

Color
Zone::getInColor() const
{
    return m_inColor;
}

Color
Zone::getOutColor() const
{
    return m_outColor;
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

        Color inColor, outColor;
        if (zoneMetadata.find("inColor") != zoneMetadata.end())
        {
            const auto inColorMetadata = zoneMetadata.at("inColor");
            /// use the border color as the in color
            /// there is also a fill color in this section
            /// but is reportedly unused by IDAS for the forseeable future
            const auto borderColorMetadata = inColorMetadata.at("borderColor");
            inColor = Color(
                borderColorMetadata[0].get<uint8_t>(),
                borderColorMetadata[1].get<uint8_t>(),
                borderColorMetadata[2].get<uint8_t>(),
                borderColorMetadata[3].get<uint8_t>()
            );
        }
        
        if (zoneMetadata.find("outColor") != zoneMetadata.end())
        {
            const auto outColorMetadata = zoneMetadata.at("outColor");
            /// use the border color as the out color
            /// there is also a fill color in this section
            /// but is reportedly unused by IDAS for the forseeable future
            const auto borderColorMetadata = outColorMetadata.at("borderColor");
            outColor = Color(
                borderColorMetadata[0].get<uint8_t>(),
                borderColorMetadata[1].get<uint8_t>(),
                borderColorMetadata[2].get<uint8_t>(),
                borderColorMetadata[3].get<uint8_t>()
            );
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
                angle,
                inColor,
                outColor
            )
        );
    }

    return zones;
}

} // namespace isx
