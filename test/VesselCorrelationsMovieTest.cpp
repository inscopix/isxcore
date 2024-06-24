#include "isxVesselSetFactory.h"
#include "isxVesselCorrelationsMovie.h"

#include "isxTest.h"
#include "catch.hpp"

TEST_CASE("VesselCorrelationsMovieTest", "[core]")
{
    const std::string dataDirPath = g_resources["unitTestDataPath"] + "/bloodflow";

    isx::CoreInitialize();

    SECTION("invalid vessel set")
    {
        const isx::SpVesselSet_t vesselSet = isx::readVesselSet(
            dataDirPath + "/blood_flow_movie_1-VD_window1s_increment1s.isxd"
        );
        const size_t vesselId = 0;

        ISX_REQUIRE_EXCEPTION(
            isx::VesselCorrelationsMovie(vesselSet, vesselId),
            isx::ExceptionFileIO,
            "No correlation heatmaps saved to input vessel set.");
    }

    SECTION("valid vessel set")
    {
        const isx::SpVesselSet_t vesselSet = isx::readVesselSetSeries({
            dataDirPath + "/rbcv_movie_1-RBCV_microns.isxd",
            dataDirPath + "/rbcv_movie_2-RBCV_microns.isxd"
        });
        const size_t vesselId = 0;

        const isx::SpMovie_t movie = std::shared_ptr<isx::VesselCorrelationsMovie>(new isx::VesselCorrelationsMovie(vesselSet, vesselId));

        SECTION("num times - gapless")
        {
            const size_t expectedNumTimes = 11;
            REQUIRE(movie->getTimingInfo().getNumTimes() == expectedNumTimes);
        }

        SECTION("num times - individual")
        {
            const std::vector<size_t> expectedNumTimes = {6, 5};
            for (size_t i = 0; i < 2; i++)
            {
                REQUIRE(movie->getTimingInfosForSeries()[i].getNumTimes() == expectedNumTimes[i]);
            }
        }

        SECTION("num pixels")
        {
            const isx::SizeInPixels_t corrSize = vesselSet->getCorrelationSize(vesselId);
            const isx::SizeInPixels_t expectedNumPixels(corrSize.getWidth() * 3, corrSize.getHeight());
            REQUIRE(movie->getSpacingInfo().getNumPixels() == expectedNumPixels);
        }

        SECTION("get frame")
        {
            const size_t frameNumber = 0;
            const isx::SpVideoFrame_t frame = movie->getFrame(frameNumber);
            REQUIRE(frame != nullptr);
            REQUIRE(frame->getImage().getSpacingInfo() == movie->getSpacingInfo());
        }
    }
    
    isx::CoreShutdown();
}
