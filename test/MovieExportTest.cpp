#include "isxMovieFactory.h"
#include "isxMovie.h"
#include "catch.hpp"
#include "isxTest.h"
#include "isxException.h"
#include "isxProject.h"
#include "isxPathUtils.h"
#include "isxMovieNWBExporter.h"

#include "H5Cpp.h"

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
    verifyTopLevelDataSets(H5::H5File & inFile, isx::MovieNWBExporterParams & inParams)
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
    verifyTimeSeriesAttributes(H5::Group & inGroup, isx::MovieNWBExporterParams & inParams, isx::SpMovie_t inMovie)
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
    verifyVideoFrames(H5::Group & inGroup, isx::MovieNWBExporterParams & inParams, isx::SpMovie_t inMovie, double inStartTimeD)
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

    SECTION("Export F32 movie series")
    {
        // write isxds
        const auto pixelsPerFrame = int32_t(sizePixels.getWidth() * sizePixels.getHeight());
        std::vector<float> buf(pixelsPerFrame * 5);   // longest movie segment has 5 frames

        int32_t i = 0;
        for (const auto & fn: filenames)
        {
            createFrameData(buf.data(), int32_t(timingInfos[i].getNumTimes()), pixelsPerFrame, i * 5);
            writeTestF32MovieGeneric(fn, timingInfos[i], spacingInfo, buf.data());
            ++i;
        }

        // export to nwb
        std::vector<isx::SpMovie_t> movies;
        for (const auto & fn: filenames)
        {
            movies.push_back(isx::readMovie(fn));
        }
        isx::MovieNWBExporterParams params(
            movies,
            exportedNwbFileName,
            "mostest made this",
            "This was exported as part of a Mosaic 2 unit test",
            "timeseries/comments",
            "timeseries/description",
            "general/experiment_description",
            "general/experimenter",
            "general/institution",
            "general/lab",
            "general/session_id");
        isx::runMovieNWBExporter(params);
        
        
        // verify exported data
        {
            H5::H5File h5File(exportedNwbFileName, H5F_ACC_RDONLY);
            verifyTopLevelDataSets(h5File, params);
            verifyTopLevelGroups(h5File);

            auto an = h5File.openGroup(S_analysis);
            REQUIRE(an.getNumAttrs() == 0);

            auto startTimeD = timingInfos[0].getStart().getSecsSinceEpoch().toDouble();

            for (const auto & fn : filenames)
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

    }

    for (const auto & fn: filenames)
    {
        std::remove(fn.c_str());
    }
}

TEST_CASE("MovieTimestampExportTest", "[core]")
{
    isx::CoreInitialize();

    const std::string outputFilename = g_resources["unitTestDataPath"] + "/test.csv";

    SECTION("isxd movie with no frame timestamps")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/cnmfe-cpp/movie_128x128x1000.isxd";
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::TSC;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilename)};

        ISX_REQUIRE_EXCEPTION(
            isx::runMovieTimestampExport(params),
            isx::ExceptionUserInput, "Input movie does not have frame timestamps stored in file.");
    }

    SECTION("invalid isxd movie series")
    {
        const std::vector<std::string> movieFilenames = {
            g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-45-49_video_sched_0_probe_custom.isxd",
            g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd",
        };
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::TSC;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilenames[0]), isx::readMovie(movieFilenames[1])};

        ISX_REQUIRE_EXCEPTION(
            isx::runMovieTimestampExport(params),
            isx::ExceptionSeries, "Members of series are not ordered in time.");
    }

    SECTION("isxd movie series with frame timestamps")
    {
        const std::vector<std::string> movieFilenames = {
            g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd",
            g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-45-49_video_sched_0_probe_custom.isxd",
        };
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::TSC;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilenames[0]), isx::readMovie(movieFilenames[1])};

        const auto status = isx::runMovieTimestampExport(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (us)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,4170546756640";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "53,1,26,4171250265074";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    SECTION("isxb movie")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb";
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::TSC;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilename)};

        const auto status = isx::runMovieTimestampExport(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (us)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,115829025489";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "112,0,112,115832757521";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    SECTION("unix timestamps")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb";
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::UNIX_EPOCH;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilename)};

        const auto status = isx::runMovieTimestampExport(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,1649819290.471000";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "112,0,112,1649819294.203032";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);        
    }

    SECTION("relative timestamps")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/20220412-200447-camera-100.isxb";
        const isx::WriteTimeRelativeTo format = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;
        isx::MovieTimestampExporterParams params(outputFilename, format);
        params.m_srcs = {isx::readMovie(movieFilename)};

        const auto status = isx::runMovieTimestampExport(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(outputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (s)";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,0.000000";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "112,0,112,3.732032";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);    
    }

    isx::CoreShutdown();
    
    std::remove(outputFilename.c_str());
}

TEST_CASE("NVisionMovieTrackingDataExportTest", "[core]")
{
    isx::CoreInitialize();

    const std::string trackingOutputFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/tracking.csv";
    const std::string zonesOutputFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/zones.csv";

    SECTION("invalid isxb movie series")
    {
        const std::vector<std::string> movieFilenames = {
            g_resources["unitTestDataPath"] +"/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_1.isxb",
            g_resources["unitTestDataPath"] +"/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb"
        };
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilenames[0]), isx::readMovie(movieFilenames[1])};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;

        ISX_REQUIRE_EXCEPTION(
            isx::runNVisionTrackingExporter(params),
            isx::ExceptionSeries, "Members of series are not ordered in time.");
    }

    SECTION("invalid dataset type")
    {
        const std::vector<std::string> movieFilenames = {
            g_resources["unitTestDataPath"] + "/baseplate/2021-06-28-23-34-09_video_sched_0_probe_none.isxd"
        };
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilenames[0])};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;

        ISX_REQUIRE_EXCEPTION(
            isx::runNVisionTrackingExporter(params),
            isx::ExceptionUserInput, "Input movie is not isxb.");
    }

    SECTION("isxb movie with tracking data - export tracking data only")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb";
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilename)};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::TSC;

        const auto status = isx::runNVisionTrackingExporter(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(trackingOutputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (us),Bounding Box Left,Bounding Box Top,Bounding Box Right,Bounding Box Bottom,Bounding Box Center X,Bounding Box Center Y,Confidence,Zone ID,Zone Name";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,163957519943,526.136,682.003,650.985,908.188,588.561,795.096,67.986,,";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "9,0,9,163957819928,524.127,642.998,649.235,884.224,586.681,763.611,95.4342,4270701760,ZONE#1 rectangle";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    SECTION("isxb movie with tracking data - export tracking data only - time since start")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb";
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilename)};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::FIRST_DATA_ITEM;

        const auto status = isx::runNVisionTrackingExporter(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(trackingOutputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (s),Bounding Box Left,Bounding Box Top,Bounding Box Right,Bounding Box Bottom,Bounding Box Center X,Bounding Box Center Y,Confidence,Zone ID,Zone Name";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,0.000000,526.136230,682.003479,650.984802,908.188293,588.560547,795.095886,67.986031,,";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "9,0,9,0.299985,524.126526,642.998047,649.234924,884.224304,586.680725,763.611206,95.434235,4270701760,ZONE#1 rectangle";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    SECTION("isxb movie with tracking data - export tracking data only - epoch time")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb";
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilename)};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::UNIX_EPOCH;

        const auto status = isx::runNVisionTrackingExporter(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(trackingOutputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (s),Bounding Box Left,Bounding Box Top,Bounding Box Right,Bounding Box Bottom,Bounding Box Center X,Bounding Box Center Y,Confidence,Zone ID,Zone Name";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,1705049721.643000,526.136230,682.003479,650.984802,908.188293,588.560547,795.095886,67.986031,,";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "9,0,9,1705049721.942985,524.126526,642.998047,649.234924,884.224304,586.680725,763.611206,95.434235,4270701760,ZONE#1 rectangle";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }
    
    SECTION("isxb movie with tracking data - export zone data only")
    {
        const std::string movieFilename = g_resources["unitTestDataPath"] + "/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb";
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilename)};
        params.m_zonesOutputFilename = zonesOutputFilename;

        const auto status = isx::runNVisionTrackingExporter(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(zonesOutputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "ID,Enabled,Name,Description,Type,X 0,Y 0,X 1,Y 1,X 2,Y 2,X 3,Y 3,X 4,Y 4,Major Axis, Minor Axis, Angle";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "1705077750976,1,ZONE#1 rectangle,,rectangle,534.135,387.9,993.203,387.9,993.203,868.86,534.135,868.86,,,,,";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "1705077943271,1,ZONE#4 Elipse,,ellipse,1273.26,241.02,,,,,,,,,293.76,98.1654,90";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    SECTION("isxb movie series with tracking data - export tracking data only")
    {
        const std::vector<std::string> movieFilenames = {
            g_resources["unitTestDataPath"] +"/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_0.isxb",
            g_resources["unitTestDataPath"] +"/nVision/tracking/Group-20240111-080531_2024-01-12-08-55-21_sched_1.isxb"
        };
        isx::NVisionMovieTrackingExporterParams params;
        params.m_srcs = {isx::readMovie(movieFilenames[0]), isx::readMovie(movieFilenames[1])};
        params.m_frameTrackingDataOutputFilename = trackingOutputFilename;
        params.m_writeTimeRelativeTo = isx::WriteTimeRelativeTo::TSC;

        const auto status = isx::runNVisionTrackingExporter(params);
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);

        std::ifstream strm(trackingOutputFilename);
        std::unique_ptr<char[]> buf = nullptr;

        const std::string expectedHeader = "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp (us),Bounding Box Left,Bounding Box Top,Bounding Box Right,Bounding Box Bottom,Bounding Box Center X,Bounding Box Center Y,Confidence,Zone ID,Zone Name";
        buf = std::unique_ptr<char[]>(new char[expectedHeader.size() + 1]);   // account for null termination
        strm.getline(buf.get(), expectedHeader.size() + 1);
        const std::string actualHeader(buf.get());
        REQUIRE(actualHeader == expectedHeader);

        const std::string expectedFirstLine = "0,0,0,163957519943,526.136,682.003,650.985,908.188,588.561,795.096,67.986,,";
        buf = std::unique_ptr<char[]>(new char[expectedFirstLine.size() + 1]);
        strm.getline(buf.get(), expectedFirstLine.size() + 1);
        const std::string actualFirstLine(buf.get());
        REQUIRE(actualFirstLine == expectedFirstLine);

        const std::string expectedLastLine = "19,1,9,163958151927,528.115,776.797,699.851,912.584,613.983,844.69,98.4992,4270701760,ZONE#1 rectangle";
#if ISX_OS_WIN32
        strm.seekg(-int64_t(expectedLastLine.size() + 2), std::ios_base::end);
#else
        strm.seekg(-int64_t(expectedLastLine.size() + 1), std::ios_base::end);
#endif
        buf = std::unique_ptr<char[]>(new char[expectedLastLine.size() + 1]);
        strm.getline(buf.get(), expectedLastLine.size() + 1);
        const std::string actualLastLine(buf.get());
        REQUIRE(actualLastLine == expectedLastLine);
    }

    isx::CoreShutdown();
    
    std::remove(trackingOutputFilename.c_str());
    std::remove(zonesOutputFilename.c_str());
}