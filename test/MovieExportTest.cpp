#include "isxMovieFactory.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxProject.h"
#include "isxPathUtils.h"

#include "isxMovieExporter.h"

#include "H5Cpp.h"
#include "isxTiffMovie.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <array>
#include <cmath>


namespace
{
    
    const char * NWB_SPEC_VERSION       = "NWB-1.0.6";
    
    const char * S_acquisition          = "acquisition";
    const char * S_analysis             = "analysis";
    const char * S_epochs               = "epochs";
    const char * S_general              = "general";
    const char * S_processing           = "processing";
    const char * S_stimulus             = "stimulus";
    const char * S_images               = "images";
    const char * S_timeseries           = "timeseries";
    const char * S_tags                 = "tags";
    const char * S_presentation         = "presentation";
    const char * S_templates            = "templates";
    const char * S_nwb_version          = "nwb_version";
    const char * S_file_create_date     = "file_create_date";
    const char * S_identifier           = "identifier";
    const char * S_session_description  = "session_description";
    const char * S_session_start_time   = "session_start_time";
    const char * S_ancestry             = "ancestry";
    const char * S_neurodata_type       = "neurodata_type";
    const char * S_source               = "source";
    const char * S_num_samples          = "num_samples";
    const char * S_bits_per_pixel       = "bits_per_pixel";
    const char * S_data                 = "data";
    const char * S_conversion           = "conversion";
    const char * S_resolution           = "resolution";
    const char * S_unit                 = "unit";
    const char * S_dimension            = "dimension";
    const char * S_format               = "format";
    const char * S_timestamps           = "timestamps";
    const char * S_interval             = "interval";
    const char * S_TimeSeries           = "TimeSeries";
    const char * S_ImageSeries          = "ImageSeries";
    const char * S_Seconds              = "Seconds";
    const char * S_raw                  = "raw";
    const char * S_None                 = "None";
    const char * S_experimenter         = "experimenter";
    const char * S_institution          = "institution";
    const char * S_lab                  = "lab";
    const char * S_session_id           = "session_id";
    const char * S_comments             = "comments";
    const char * S_description          = "description";
    const char * S_experiment_description = "experiment_description";

    bool
    verifyH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, const char *  inExpectedValue)
    {
        auto attr = inH5Loc.openAttribute(inName);
        auto tid = H5::StrType(0, H5T_VARIABLE);
        const char * actual;
        attr.read(tid, &actual);
        attr.close();
        return std::string(actual) == std::string(inExpectedValue);
    }

    bool
    verifyH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, int32_t inExpectedValue)
    {
        auto attr = inH5Loc.openAttribute(inName);
        int32_t actual = -1;
        attr.read(H5::PredType::STD_I32LE, &actual);
        attr.close();
        return actual == inExpectedValue;
    }
    
    bool
    verifyH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, float inExpectedValue)
    {
        auto attr = inH5Loc.openAttribute(inName);
        float actual = -1.f;
        attr.read(H5::PredType::IEEE_F32LE, &actual);
        attr.close();
        if (actual == inExpectedValue)
        {
            return true;
        }

        return fabs(actual - inExpectedValue) < 0.0001;
    }

    void
    verifyDataSet(H5::CommonFG & inCommonFG, const char * inDataSetName, const std::string & inStringVal)
    {
        bool didThrow = false;
        try
        {
            auto tidStr = H5::StrType(0, H5T_VARIABLE);
            auto ds = inCommonFG.openDataSet(inDataSetName);
            std::string actual;
            ds.read(actual, tidStr);
            if (inStringVal == "")
            {
                REQUIRE(actual != "");
            }
            else
            {
                REQUIRE(actual == inStringVal);
            }
            ds.close();
        }
        catch (H5::Exception &/*e*/)
        {
            auto i = std::string("Dataset missing ") + inDataSetName;
            FAIL(i);
            didThrow = true;
        }
        REQUIRE(didThrow == false);
    }

    bool
    verifyDataSet(H5::CommonFG & inCommonFG, const std::string & inName, int32_t inExpectedValue)
    {
        auto ds = inCommonFG.openDataSet(inName);
        int32_t actual = -1;
        ds.read(&actual, H5::PredType::STD_I32LE);
        ds.close();
        return actual == inExpectedValue;
    }
    
    void
    verifyTopLevelDataSets(H5::H5File & inFile, isx::MovieExporterParams & inParams)
    {
        verifyDataSet(inFile, S_nwb_version, NWB_SPEC_VERSION);
        verifyDataSet(inFile, S_file_create_date, "");
        verifyDataSet(inFile, S_identifier, inParams.m_identifier);
        verifyDataSet(inFile, S_session_description, inParams.m_sessionDescription);
        auto startTime = inParams.m_srcs[0]->getTimingInfo().getStart();
        verifyDataSet(inFile, S_session_start_time, startTime.getAsIso8601String());
    }
    
    void
    verifyGroup(H5::CommonFG & inCommonFG, const char * inGroupName, int32_t inNumAttr = 0)
    {
        bool didThrow = false;
        try
        {
            auto g = inCommonFG.openGroup(inGroupName);
            REQUIRE(g.getNumAttrs() == inNumAttr);
            g.close();
        }
        catch (H5::Exception &/*e*/)
        {
            auto i = std::string("Group missing: ") + inGroupName;
            FAIL(i);
            didThrow = true;
        }
        REQUIRE(didThrow == false);
    }
    
    void
    verifyTopLevelGroups(H5::H5File & inFile)
    {
        verifyGroup(inFile, S_acquisition);
        auto g = inFile.openGroup(S_acquisition);
        {
            verifyGroup(g, S_images);
            verifyGroup(g, S_timeseries);
        }
        g.close();

        verifyGroup(inFile, S_analysis);
        verifyGroup(inFile, S_epochs, 1);
        g = inFile.openGroup(S_epochs);
        auto attr = g.openAttribute(S_tags);
        REQUIRE(attr.getTypeClass() == H5T_STRING);
        attr.close();
        g.close();
        verifyGroup(inFile, S_general);
        g = inFile.openGroup(S_general);
        {
            verifyDataSet(g, S_experiment_description, "general/experiment_description");
            verifyDataSet(g, S_experimenter, "general/experimenter");
            verifyDataSet(g, S_institution, "general/institution");
            verifyDataSet(g, S_lab, "general/lab");
            verifyDataSet(g, S_session_id, "general/session_id");
        }
        g.close();
        verifyGroup(inFile, S_processing);
        verifyGroup(inFile, S_stimulus);
        g = inFile.openGroup(S_stimulus);
        {
            verifyGroup(g, S_presentation);
            verifyGroup(g, S_templates);
        }
        g.close();
    }

    void
    verifyTimeSeriesAttributes(H5::Group & inGroup, isx::MovieExporterParams & inParams, isx::SpMovie_t inMovie)
    {
        auto attr = inGroup.openAttribute(S_ancestry);
        auto dt = attr.getDataType();
        const char * ancBuf[2];
        attr.read(dt, ancBuf);
        attr.close();
        REQUIRE(std::string(ancBuf[0]) == S_TimeSeries);
        REQUIRE(std::string(ancBuf[1]) == S_ImageSeries);

        verifyH5Attribute(inGroup, S_neurodata_type, S_TimeSeries);
        std::string dataSource = inMovie->getFileName();
        auto pos1 = dataSource.rfind('/');
        auto pos2 = dataSource.rfind('/', pos1 - 1);
        auto pos3 = dataSource.rfind('/', pos2 - 1);
        auto pos = std::min(pos1, std::min(pos2, pos3));
        dataSource = dataSource.substr(pos);
        
        verifyH5Attribute(inGroup, S_source, dataSource.c_str());
        int32_t numSamples = int32_t(inMovie->getTimingInfo().getNumValidTimes());
        verifyDataSet(inGroup, S_num_samples, numSamples);
        int32_t bitsPerPixel = 8 * int32_t(getDataTypeSizeInBytes(inMovie->getDataType()));
        verifyDataSet(inGroup, S_bits_per_pixel, bitsPerPixel);
        
        {
            auto ds = inGroup.openDataSet(S_dimension);
            dt = ds.getDataType();
            int32_t dimBuf[2];
            ds.read(dimBuf, dt);
            ds.close();
            REQUIRE(dimBuf[0] == int32_t(inMovie->getSpacingInfo().getNumColumns()));
            REQUIRE(dimBuf[1] == int32_t(inMovie->getSpacingInfo().getNumRows()));
        }

        verifyDataSet(inGroup, S_format, S_raw);
        verifyH5Attribute(inGroup, S_comments, "timeseries/comments");
        verifyH5Attribute(inGroup, S_description, "timeseries/description");
    }

    void
    verifyVideoFrames(H5::Group & inGroup, isx::MovieExporterParams & inParams, isx::SpMovie_t inMovie, double inStartTimeD)
    {
        auto width  = hsize_t(inMovie->getSpacingInfo().getNumColumns());
        auto height = hsize_t(inMovie->getSpacingInfo().getNumRows());
        auto numValidFrames = hsize_t(inMovie->getTimingInfo().getNumValidTimes());
        
        auto data = inGroup.openDataSet(S_data);
        auto dataSpace = data.getSpace();
        std::array<hsize_t, 3> dimsMovieExpected{ {numValidFrames, height, width} };
        std::array<hsize_t, 3> dimsMovieActual;
        REQUIRE(dataSpace.getSimpleExtentNdims() == 3);
        REQUIRE(data.getDataType() == H5::PredType::IEEE_F32LE);
        dataSpace.getSimpleExtentDims(dimsMovieActual.data());
        REQUIRE(dimsMovieActual[0] == dimsMovieExpected[0]);
        REQUIRE(dimsMovieActual[1] == dimsMovieExpected[1]);
        REQUIRE(dimsMovieActual[2] == dimsMovieExpected[2]);
        verifyH5Attribute(data, S_conversion, 1.f);
        verifyH5Attribute(data, S_resolution, sqrtf(-1.f));
        verifyH5Attribute(data, S_unit, S_None);
        
        auto timeStamps = inGroup.openDataSet(S_timestamps);
        auto timeStampsSpace = timeStamps.getSpace();
        std::array<hsize_t, 1> dimsTimeStampsExpected{ {numValidFrames} };
        std::array<hsize_t, 1> dimsTimeStampsActual;
        REQUIRE(timeStampsSpace.getSimpleExtentNdims() == 1);
        REQUIRE(timeStamps.getDataType() == H5::PredType::IEEE_F64LE);
        timeStampsSpace.getSimpleExtentDims(dimsTimeStampsActual.data());
        REQUIRE(dimsTimeStampsActual[0] == dimsTimeStampsExpected[0]);
        verifyH5Attribute(timeStamps, S_interval, int32_t(1));
        verifyH5Attribute(timeStamps, S_unit, S_Seconds);

        auto numSamples = hsize_t(inMovie->getTimingInfo().getNumTimes());
        
        const auto pixelsPerFrame = dimsMovieActual[1] * dimsMovieActual[2];
        std::unique_ptr<float[]> frameDataActual(new float[pixelsPerFrame]);
        for (hsize_t i = 0; i < numSamples; ++i)
        {
            auto f = inMovie->getFrame(i);
            if (f->getFrameType() == isx::VideoFrame::Type::VALID)
            {
                auto readIndex = hsize_t(inMovie->getTimingInfo().timeIdxToRecordedIdx(i));
                
                // data
                {
                    auto fileSpace = dataSpace;
                    hsize_t fileStart[3] = {readIndex, 0, 0};
                    hsize_t fileCount[3] = {1, dimsMovieActual[1], dimsMovieActual[2]};
                    fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
                    
                    H5::DataSpace bufferSpace(3, fileCount);
                    hsize_t bufferStart[3] = { 0, 0, 0 };
                    bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);

                    data.read(frameDataActual.get(), H5::PredType::IEEE_F32LE, bufferSpace, fileSpace);
                    REQUIRE(0 == std::memcmp(frameDataActual.get(), f->getPixels(), pixelsPerFrame * sizeof(float)));
                }
                
                // timestamp
                {
                    auto fileSpace = timeStampsSpace;
                    hsize_t fileStart[1] = {readIndex};
                    hsize_t fileCount[1] = {1};
                    fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
                    
                    H5::DataSpace bufferSpace(1, fileCount);
                    hsize_t bufferStart[1] = { 0 };
                    bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);
                    
                    auto timeStampExpected = f->getTimeStamp().getSecsSinceEpoch().toDouble() - inStartTimeD;
                    double timeStampActual = -1.0;
                    timeStamps.read(&timeStampActual, H5::PredType::IEEE_F64LE, bufferSpace, fileSpace);
                    REQUIRE(timeStampActual == timeStampExpected);
                }
            }
        }

        timeStamps.close();
        data.close();
    }

    void
    createFrameData(float * outBuf, int32_t inNumFrames, int32_t inPixelsPerFrame, int32_t inBaseValueForFrame)
    {
        for (int32_t f = 0; f < inNumFrames; ++f)
        {
            for (int32_t p = 0; p < inPixelsPerFrame; ++p)
            {
                outBuf[f * inPixelsPerFrame + p] = float((inBaseValueForFrame + f) * inPixelsPerFrame + p);
            }
        }
    }

    void
    createFrameData(uint16_t * outBuf, int32_t inNumFrames, int32_t inPixelsPerFrame, int32_t inBaseValueForFrame)
    {
        for (int32_t f = 0; f < inNumFrames; ++f)
        {
            for (int32_t p = 0; p < inPixelsPerFrame; ++p)
            {
                outBuf[f * inPixelsPerFrame + p] = uint16_t((inBaseValueForFrame + f) * inPixelsPerFrame + p);
            }
        }
    }

} // namespace

    
TEST_CASE("MovieExportTest", "[core]")
{
    std::array<const char *, 3> names =
    { {
        "seriesMovie0.isxd",
        "seriesMovie1.isxd",
        "seriesMovie2.isxd"
    } };
    std::vector<std::string> filenames;

    for (auto n: names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }
    
    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }

    std::string exportedNwbFileName = g_resources["unitTestDataPath"] + "/exportedMovie.nwb";
    std::remove(exportedNwbFileName.c_str());

    std::string exportedTiffFileName = g_resources["unitTestDataPath"] + "/exportedMovie.tiff";
    std::remove(exportedTiffFileName.c_str());

    std::vector<isx::isize_t> dropped = { 2 };
    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5, dropped }
    } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::DataType dataType = isx::DataType::U16;

    isx::CoreInitialize();

    SECTION("Export movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto fn: filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto fn: filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieExporterParams params(
            movies,
            exportedNwbFileName,
            exportedTiffFileName,
            "mostest made this",
            "This was exported as part of a Mosaic 2 unit test",
            "timeseries/comments",
            "timeseries/description",
            "general/experiment_description",
            "general/experimenter",
            "general/institution",
            "general/lab",
            "general/session_id");
        isx::runMovieExporter(params, nullptr, [](float){return false;});
        
        
        // verify exported data
        {
            H5::H5File h5File(exportedNwbFileName, H5F_ACC_RDONLY);
            verifyTopLevelDataSets(h5File, params);
            verifyTopLevelGroups(h5File);

            auto an = h5File.openGroup(S_analysis);
            REQUIRE(an.getNumAttrs() == 0);

            auto startTimeD = timingInfos[0].getStart().getSecsSinceEpoch().toDouble();

            for (const auto fn : filenames)
            {
                auto name = isx::getBaseName(fn);
                auto g = an.openGroup(name);
                REQUIRE(g.getNumAttrs() == 5);
                auto m = isx::readMovie(fn);
                verifyTimeSeriesAttributes(g, params, m);
                verifyVideoFrames(g, params, m, startTimeD);
            }

            h5File.close();
        }

        {
            //isx::TiffMovie tiffMovie(exportedTiffFileName);

        }

    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieExportTiffU16Test", "[core]")
{
    std::array<const char *, 3> names =
    { {
            "seriesMovie0.isxd",
            "seriesMovie1.isxd",
            "seriesMovie2.isxd"
        } };
    std::vector<std::string> filenames;

    for (auto n : names)
    {
        filenames.push_back(g_resources["unitTestDataPath"] + "/" + n);
    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }

    std::string exportedNwbFileName = g_resources["unitTestDataPath"] + "/exportedMovie.nwb";
    std::remove(exportedNwbFileName.c_str());

    std::string exportedTiffFileName = g_resources["unitTestDataPath"] + "/exportedMovie.tiff";
    std::remove(exportedTiffFileName.c_str());

    std::vector<isx::isize_t> dropped = { 2 };
    std::array<isx::TimingInfo, 3> timingInfos =
    { {
        { isx::Time(2222, 4, 1, 3, 0, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 3 },
        { isx::Time(2222, 4, 1, 3, 1, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 4 },
        { isx::Time(2222, 4, 1, 3, 2, 0, isx::DurationInSeconds(0, 1)), isx::Ratio(1, 20), 5, dropped }
        } };

    isx::SizeInPixels_t sizePixels(4, 3);
    isx::SpacingInfo spacingInfo(sizePixels);

    isx::CoreInitialize();

    SECTION("Export movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<uint16_t> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto fn : filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestU16MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto fn : filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieExporterParams params(
            movies,
            exportedNwbFileName,
            exportedTiffFileName,
            "mostest made this",
            "This was exported as part of a Mosaic 2 unit test",
            "timeseries/comments",
            "timeseries/description",
            "general/experiment_description",
            "general/experimenter",
            "general/institution",
            "general/lab",
            "general/session_id");
        isx::runMovieExporter(params, nullptr, [](float) {return false; });


        // verify exported data
        {
            isx::TiffMovie tiffMovie(exportedTiffFileName);
            REQUIRE(tiffMovie.getFrameHeight() == sizePixels.getHeight());
            REQUIRE(tiffMovie.getFrameWidth() == sizePixels.getWidth());
            REQUIRE(tiffMovie.getDataType() == isx::DataType::U16);
        }

    }

    for (const auto & fn : filenames)
    {
        std::remove(fn.c_str());
    }
}
