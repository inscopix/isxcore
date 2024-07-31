#include "catch.hpp"
#include "isxTest.h"

#include "isxNVisionTracking.h"
#include "isxMovieFactory.h"


TEST_CASE("NVisionTrackingBoundingBox", "[core]")
{
    isx::CoreInitialize();

    SECTION("From metadata - single zone event")
    {
        const std::string frameMetadata = "{\"fc\":19,\"tracker\":{\"box\":[528.115173339844,776.796661376953,699.851165771484,912.584289550781],\"conf\":98.4991912841797,\"zones\":{\"id\":4270701760,\"name\":\"ZONE#1 rectangle\"}},\"tsc\":163958151927}";

        const isx::BoundingBox expectedBoundingBox(
            776.796661376953f,
            528.115173339844f,
            912.584289550781f,
            699.851165771484f,
            98.4991912841797f,
            {
                isx::ZoneEvent(
                    4270701760,
                    "ZONE#1 rectangle",
                    isx::ZoneEvent::Type::NONE,
                    isx::ZoneEvent::Trigger::NONE
                )
            }
        );
        const auto actualBoundingBox = isx::BoundingBox::fromMetadata(frameMetadata);
        REQUIRE(expectedBoundingBox == actualBoundingBox);
    }

    SECTION("From metadata - multi zone event")
    {
        const std::string frameMetadata = "{\"fc\":2844,\"tracker\":{\"box\":[1268.00234985352,768.432312011719,1365.31988525391,895.73291015625],\"conf\":80.8296203613281,\"zones\":[{\"event\":\"e\",\"id\":1720032796530,\"name\":\"ZONE#3\",\"trig\":\"softTrig-3\"},{\"event\":\"x\",\"id\":1720032411965,\"name\":\"ZONE#2\",\"trig\":\"softTrig-2\"}]},\"tsc\":1017645148394}";

        const isx::BoundingBox expectedBoundingBox(
            768.432312011719f,
            1268.00234985352f,
            895.73291015625f,
            1365.31988525391f,
            80.8296203613281f,
            {
                isx::ZoneEvent(
                    1720032796530,
                    "ZONE#3",
                    isx::ZoneEvent::Type::ENTRY,
                    isx::ZoneEvent::Trigger::SOFT_TRIG_3
                ),
                isx::ZoneEvent(
                    1720032411965,
                    "ZONE#2",
                    isx::ZoneEvent::Type::EXIT,
                    isx::ZoneEvent::Trigger::SOFT_TRIG_2
                )
            }
        );
        const auto actualBoundingBox = isx::BoundingBox::fromMetadata(frameMetadata);
        REQUIRE(expectedBoundingBox == actualBoundingBox);
    }

    isx::CoreShutdown();
}

TEST_CASE("NVisionTrackingZones", "[core]")
{
    isx::CoreInitialize();

    SECTION("From metadata")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] +"/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_1.isxb";
        const auto movie = isx::readMovie(movieFilename);
        const auto extraProperties = movie->getExtraProperties();
        const std::vector<isx::Zone> expectedZones = {
            isx::Zone(
                1705077750976,
                true,
                "ZONE#1 rectangle",
                "",
                isx::Zone::Type::RECTANGLE,
                std::vector<isx::SpatialPoint<float>>({
                    isx::SpatialPoint<float>(
                        534.135338345865f,
                        387.9f
                    ),
                    isx::SpatialPoint<float>(
                        993.203007518797f,
                        387.9f
                    ),
                    isx::SpatialPoint<float>(
                        993.203007518797f,
                        868.86f
                    ),
                    isx::SpatialPoint<float>(
                        534.135338345865f,
                        868.86f
                    )
                }),
                0.0f, 0.0f, 0.0f,
                isx::Color(255, 0, 0, 255),
                isx::Color(255, 165, 0, 255)
            ),
            isx::Zone(
                1705077787809,
                true,
                "ZONE#2 Elipse",
                "",
                isx::Zone::Type::ELLIPSE,
                std::vector<isx::SpatialPoint<float>>({
                    isx::SpatialPoint<float>(
                        884.932330827068f,
                        226.62f
                    )
                }),
                568.781954887218f,
                270.72f,
                0.0f,
                isx::Color(255, 0, 0, 255),
                isx::Color(0,0,255,255)
            ),
            isx::Zone(
                1705077917385,
                true,
                "ZONE#3 Polygon",
                "",
                isx::Zone::Type::POLYGON,
                std::vector<isx::SpatialPoint<float>>({
                    isx::SpatialPoint<float>(
                        1019.18796992481f,
                        403.2f
                    ),
                    isx::SpatialPoint<float>(
                        1417.62406015038f,
                        402.3f
                    ),
                    isx::SpatialPoint<float>(
                        1408.96240601504f,
                        912.06f
                    ),
                    isx::SpatialPoint<float>(
                        1120.24060150376f,
                        891.9f
                    ),
                    isx::SpatialPoint<float>(
                        1022.07518796992f,
                        771.84f
                    ),
                }),
                0.0f, 0.0f, 0.0f,
                isx::Color(255, 0, 0, 255),
                isx::Color(255,255,0,255)
            ),
            isx::Zone(
                1705077943271,
                true,
                "ZONE#4 Elipse",
                "",
                isx::Zone::Type::ELLIPSE,
                std::vector<isx::SpatialPoint<float>>({
                    isx::SpatialPoint<float>(
                        1273.26315789474f,
                        241.02f
                    )
                }),
                293.76f,
                98.1654135338346f,
                90.0f,
                isx::Color(255, 0, 0, 255),
                isx::Color(255,0,255,255)
            ),
        };
        
        const auto actualZones = isx::getZonesFromMetadata(extraProperties);
        for (size_t i = 0; i < 4; i++)
        {
            REQUIRE(actualZones[i] == expectedZones[i]);
        }
    }

    isx::CoreShutdown();
}
