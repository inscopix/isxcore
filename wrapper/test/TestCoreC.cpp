#include "catch.hpp"
#include "isxTest.h"

extern "C"
{
#include "isxCoreC.h"
}

#include "stdio.h"
#include "json.hpp"
#include <stdexcept>

TEST_CASE("IsxMovie-read", "[corec]")
{

    REQUIRE(isx_initialize() == 0);

    SECTION("Empty file")
    {
        const std::string file_path = g_resources["unitTestDataPath"] + "/negative/empty.isxd";
        IsxMovie * movie = nullptr;
        REQUIRE(isx_read_movie(file_path.c_str(), &movie) != 0);
    }

    REQUIRE(isx_shutdown() == 0);
}

TEST_CASE("IsxMovie-write_then_read", "[corec]")
{
    const std::string file_path = g_resources["unitTestDataPath"] + "/testcorec-IsxMovie-write_then_read.isxd";
    remove(file_path.c_str());

    REQUIRE(isx_initialize() == 0);

    const size_t max_num_dropped = 2048;

    SECTION("Movie with u16 pixel values, frame header/footer and extra properties")
    {
        // Define the properties of the movie, though the number of frames
        // (num_samples) is defined after we write the frames to simulate acquisition.
        IsxTimingInfo ti;
        ti.num_samples = 0;
        ti.start.secs_since_epoch.num = 1521586282;
        ti.start.secs_since_epoch.den = 1000;
        ti.start.utc_offset = 0;
        ti.step.num = 50;
        ti.step.den = 1000;
        ti.num_dropped = 0;
        ti.dropped = new size_t[max_num_dropped];
        ti.num_cropped = 0;
        ti.cropped_first = nullptr;
        ti.cropped_last = nullptr;

        IsxSpacingInfo si;

        SECTION("1280x800 - full resolution")
        {
            si.num_rows = 800;
            si.num_cols = 1280;
            si.pixel_width.num = 3;
            si.pixel_height.num = 3;
        }

        SECTION("640x400 - downsampled by a factor of 2")
        {
            si.num_rows = 400;
            si.num_cols = 640;
            si.pixel_width.num = 6;
            si.pixel_height.num = 6;
        }

        SECTION("582x333 - downsampled by a factor of 2 and cropped")
        {
            si.num_rows = 333;
            si.num_cols = 582;
            si.pixel_width.num = 6;
            si.pixel_height.num = 6;
        }

        SECTION("426x266 - downsampled by a factor of 3")
        {
            si.num_rows = 266;
            si.num_cols = 426;
            si.pixel_width.num = 9;
            si.pixel_height.num = 9;
        }

        SECTION("320x200 - downsampled by a factor of 4")
        {
            si.num_rows = 200;
            si.num_cols = 320;
            si.pixel_width.num = 12;
            si.pixel_height.num = 12;
        }

        si.pixel_width.den = 1;
        si.pixel_height.den = 1;
        si.left.num = 0;
        si.left.den = 1;
        si.top.num = 0;
        si.top.den = 1;
        const size_t num_pixels = si.num_rows * si.num_cols;

        using json = nlohmann::json;
        json extra_properties_j;
        extra_properties_j["probe"]["name"] = "ISX3821092";
        extra_properties_j["probe"]["type"] = "straight";
        extra_properties_j["probe"]["length"] = 8;
        extra_properties_j["probe"]["diameter"] = 0.6;
        extra_properties_j["microscope"]["EX-LED power"] = 9000;
        extra_properties_j["microscope"]["Spatial downsample"] = 2;
        extra_properties_j["microscope"]["FOV"]["width"] = si.num_cols;
        extra_properties_j["microscope"]["FOV"]["height"] = si.num_rows;
        const std::string extra_properties = extra_properties_j.dump();

        // Write the movie.
        IsxMovie * movie = nullptr;
        REQUIRE(isx_write_movie(file_path.c_str(), ti, si, isx_get_data_type_u16(), true, &movie) == 0);
        REQUIRE(isx_movie_set_extra_properties(movie, extra_properties.c_str()) == 0);

        const size_t num_header_values = 2 * 1280;
        const size_t num_footer_values = num_header_values;

        const uint16_t header_value = 137;
        const uint16_t footer_value = 92;

        const uint16_t max_frame_value = 4095;

        std::unique_ptr<uint16_t[]> frame_data_with_hf(new uint16_t[num_pixels + num_header_values + num_footer_values]);
        std::unique_ptr<uint16_t[]> frame_data(new uint16_t[num_pixels]);
        std::unique_ptr<uint16_t[]> frame_header(new uint16_t[num_header_values]);
        std::unique_ptr<uint16_t[]> frame_footer(new uint16_t[num_footer_values]);

        size_t f = 0;
        const size_t num_samples = 7;
        for (; f < num_samples; ++f)
        {
            if (f == 1 || f == 4)
            {
                if (ti.num_dropped >= max_num_dropped)
                {
                    throw std::length_error("Dropped too many frames (" + std::to_string(ti.num_dropped) + "). Giving up.");
                }
                ti.dropped[ti.num_dropped++] = f;
                continue;
            }

            if ((si.num_cols == 1280 && si.num_rows == 800))
            {
                size_t p = 0;
                for (; p < num_header_values; ++p)
                {
                    frame_data_with_hf[p] = header_value;
                }
                for (; p < num_header_values + num_pixels; ++p)
                {
                    frame_data_with_hf[p] = (uint16_t)hashFrameAndPixelIndex(f, p - num_header_values, max_frame_value);
                }
                for (; p < num_header_values + num_pixels + num_footer_values; ++p)
                {
                    frame_data_with_hf[p] = footer_value;
                }
                REQUIRE(isx_movie_write_frame_u16_with_header_footer(movie, f, frame_data_with_hf.get()) == 0);
            }
            else
            {
                for (size_t p = 0; p < num_header_values; ++p)
                {
                    frame_header[p] = header_value;
                }
                for (size_t p = 0; p < num_pixels; ++p)
                {
                    frame_data[p] = (uint16_t)hashFrameAndPixelIndex(f, p, max_frame_value);
                }
                for (size_t p = 0; p < num_footer_values; ++p)
                {
                    frame_footer[p] = footer_value;
                }
                REQUIRE(isx_movie_write_frame_u16_with_header_footer_separately(
                            movie, frame_header.get(), frame_data.get(), frame_footer.get()) == 0);
            }
        }

        ti.num_samples = num_samples;

        REQUIRE(isx_movie_flush_with_timing_info(movie, ti) == 0);
        REQUIRE(isx_movie_delete(movie) == 0);

        // Read the movie back in and verify.
        movie = nullptr;
        REQUIRE(isx_read_movie(file_path.c_str(), &movie) == 0);

        REQUIRE(movie != nullptr);

        REQUIRE(movie->timing.start.secs_since_epoch.num == ti.start.secs_since_epoch.num);
        REQUIRE(movie->timing.start.secs_since_epoch.den == ti.start.secs_since_epoch.den);
        REQUIRE(movie->timing.start.utc_offset == ti.start.utc_offset);
        REQUIRE(movie->timing.step.den == ti.step.den);
        REQUIRE(movie->timing.step.den == ti.step.den);
        REQUIRE(movie->timing.num_samples == ti.num_samples);

        REQUIRE(movie->timing.num_dropped == ti.num_dropped);
        for (size_t i = 0; i < movie->timing.num_dropped; ++i)
        {
            REQUIRE(movie->timing.dropped[i] == ti.dropped[i]);
        }

        REQUIRE(movie->timing.num_cropped == ti.num_cropped);
        for (size_t i = 0; i < movie->timing.num_cropped; ++i)
        {
            REQUIRE(movie->timing.cropped_first[i] == ti.cropped_first[i]);
            REQUIRE(movie->timing.cropped_last[i] == ti.cropped_last[i]);
        }

        REQUIRE(movie->spacing.num_rows == si.num_rows);
        REQUIRE(movie->spacing.num_cols == si.num_cols);
        REQUIRE(movie->spacing.pixel_width.num == si.pixel_width.num);
        REQUIRE(movie->spacing.pixel_width.den == si.pixel_width.den);
        REQUIRE(movie->spacing.pixel_height.num == si.pixel_height.num);
        REQUIRE(movie->spacing.pixel_height.den == si.pixel_height.den);
        REQUIRE(movie->spacing.left.num == si.left.num);
        REQUIRE(movie->spacing.left.den == si.left.den);
        REQUIRE(movie->spacing.top.num == si.top.num);
        REQUIRE(movie->spacing.top.den == si.top.den);

        REQUIRE(movie->read_only == true);
        REQUIRE(movie->data_type == isx_get_data_type_u16());

        // There's almost definitely a way to improve this, but I hate C strings and
        // this is the best I've got.
        size_t actual_extra_properties_size = 0;
        REQUIRE(isx_movie_get_extra_properties_size(movie, &actual_extra_properties_size) == 0);
        std::unique_ptr<char[]> actual_extra_properties(new char[actual_extra_properties_size]);
        REQUIRE(isx_movie_get_extra_properties(movie, actual_extra_properties.get(), actual_extra_properties_size) == 0);
        REQUIRE(json::parse(actual_extra_properties.get()) == extra_properties_j);

        for (size_t f = 0; f < ti.num_samples; ++f)
        {
            if (f == 1 || f == 4)
            {
                continue;
            }

            REQUIRE(isx_movie_get_frame_data_u16(movie, f, frame_data.get()) == 0);

            for (size_t p = 0; p < num_pixels; ++p)
            {
                const uint16_t exp_value = (uint16_t)hashFrameAndPixelIndex(f, p, max_frame_value);
                if (frame_data[p] != exp_value)
                {
                    FAIL("frame_data[" << p << "] != " << exp_value);
                }
            }

            REQUIRE(isx_movie_get_frame_header(movie, f, frame_header.get()) == 0);
            for (size_t p = 0; p < num_header_values; ++p)
            {
                if (frame_header[p] != header_value)
                {
                    FAIL("frame_header[" << p << "] != " << header_value);
                }
            }

            REQUIRE(isx_movie_get_frame_footer(movie, f, frame_footer.get()) == 0);
            for (size_t p = 0; p < num_footer_values; ++p)
            {
                if (frame_footer[p] != footer_value)
                {
                    FAIL("frame_footer[" << p << "] != " << footer_value);
                }
            }

            REQUIRE(isx_movie_get_frame_data_u16_with_header_footer(movie, f, frame_data_with_hf.get()) == 0);
            size_t p = 0;
            for (; p < num_header_values; ++p)
            {
                if (frame_data_with_hf[p] != header_value)
                {
                    FAIL("frame_data_with_hf[" << p << "] == " << frame_data_with_hf[p] << " != " << header_value);
                }
            }
            for (; p < num_header_values + num_pixels; ++p)
            {
                const uint16_t exp_value = (uint16_t)hashFrameAndPixelIndex(f, p - num_header_values, max_frame_value);
                if (frame_data_with_hf[p] != exp_value)
                {
                    FAIL("frame_data_with_hf[" << p << "] == " << frame_data_with_hf[p] << " != " << exp_value);
                }
            }
            for (; p < num_header_values + num_pixels + num_footer_values; ++p)
            {
                if (frame_data_with_hf[p] != footer_value)
                {
                    FAIL("frame_data_with_hf[" << p << "] == " << frame_data_with_hf[p] << " != " << footer_value);
                }
            }
        }

        REQUIRE(isx_movie_delete(movie) == 0);
    }

    REQUIRE(isx_shutdown() == 0);

    remove(file_path.c_str());
}
