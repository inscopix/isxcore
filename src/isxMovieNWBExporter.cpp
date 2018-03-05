#include "isxMovieExporter.h"
#include "isxMovieNWBExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxException.h"
#include "isxTime.h"
#include "isxMovie.h"

#include "H5Cpp.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <array>
#include <cmath>

#include "json.hpp"

namespace isx {

std::string
MovieNWBExporterParams::getOpName()
{
    return "Export Movie";
}

std::string
MovieNWBExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["identifier"] = m_identifier;
    j["sessionDescription"] = m_sessionDescription;
    j["comments"] = m_comments;
    j["description"] = m_description;
    j["experimentDescription"] = m_experimentDescription;
    j["experimenter"] = m_experimenter;
    j["institution"] = m_institution;
    j["lab"] = m_lab;
    j["sessionId"] = m_sessionId;
    j["filename"] = m_filename;
    return j.dump(4);
}

MovieExporterParams::Type
MovieNWBExporterParams::getType()
{
    return MovieExporterParams::Type::NWB;
}

void
MovieNWBExporterParams::setOutputFileName(const std::string & inFileName)
{
    m_filename = inFileName;
}

void
MovieNWBExporterParams::setSources(const std::vector<SpMovie_t> & inSources)
{
    m_srcs = inSources;
}

void
MovieNWBExporterParams::setAdditionalInfo(
        const std::string & inIdentifierBase,
        const std::string & inSessionDescription,
        const std::string & inComments,
        const std::string & inDescription,
        const std::string & inExperimentDescription,
        const std::string & inExperimenter,
        const std::string & inInstitution,
        const std::string & inLab,
        const std::string & inSessionId)
{
    m_identifier            = inIdentifierBase;
    m_sessionDescription    = inSessionDescription;
    m_comments              = inComments;
    m_description           = inDescription;
    m_experimentDescription = inExperimentDescription;
    m_experimenter          = inExperimenter;
    m_institution           = inInstitution;
    m_lab                   = inLab;
    m_sessionId             = inSessionId;
}

void
MovieNWBExporterParams::setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped)
{
    // Do nothing - currently NWB cannot contains these details
}

const char *
MovieNWBExporterParams::sNwbVersion = "NWB-1.0.6";

std::string
MovieNWBExporterParams::makeIdString(const std::string & inBase)
{
    const auto nowT = isx::Time::now();
    const auto nowS = nowT.getAsIso8601String();
    return inBase + " " + sNwbVersion + " " + nowS;
}
    
namespace
{
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

    void
    writeH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, const char * inValue)
    {
        auto dataSpace = H5::DataSpace();
        auto tid = H5::StrType(0, H5T_VARIABLE);

        auto attr = inH5Loc.createAttribute(inName, tid, dataSpace);
        attr.write(tid, &inValue);
        attr.close();
    }

    void
    writeH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, int32_t inValue)
    {
        auto dataSpace = H5::DataSpace();
        auto tid = H5::PredType::STD_I32LE;
        
        auto attr = inH5Loc.createAttribute(inName, tid, dataSpace);
        attr.write(tid, &inValue);
        attr.close();
    }

    void
    writeH5Attribute(H5::H5Location & inH5Loc, const std::string & inName, float inValue)
    {
        auto dataSpace = H5::DataSpace();
        auto tid = H5::PredType::IEEE_F32LE;
        
        auto attr = inH5Loc.createAttribute(inName, tid, dataSpace);
        attr.write(tid, &inValue);
        attr.close();
    }

    void
    writeH5DataSet(H5::CommonFG & inH5Grp, const std::string & inName, int32_t inValue)
    {
        auto dataSpace = H5::DataSpace();
        auto tid = H5::PredType::STD_I32LE;

        auto ds = inH5Grp.createDataSet(inName, tid, dataSpace);
        ds.write(&inValue, tid);
        ds.close();
    }

    void
    writeH5DataSet(H5::CommonFG & inH5Grp, const std::string & inName, const char * inValue)
    {
        auto dataSpace = H5::DataSpace();
        auto tid = H5::StrType(0, H5T_VARIABLE);

        auto ds = inH5Grp.createDataSet(inName, tid, dataSpace);
        ds.write(&inValue, tid);
        ds.close();
    }

    void
    writeTopLevelGroups(H5::H5File & inH5File, MovieNWBExporterParams & inParams)
    {
        auto ac = inH5File.createGroup(S_acquisition);
        auto an = inH5File.createGroup(S_analysis);
        auto ep = inH5File.createGroup(S_epochs);
        auto ge = inH5File.createGroup(S_general);
        auto pr = inH5File.createGroup(S_processing);
        auto st = inH5File.createGroup(S_stimulus);
        
        // acquisition
        auto acImages = ac.createGroup(S_images);
        auto acTimeseries = ac.createGroup(S_timeseries);
        
        // epochs
        {
            std::array<hsize_t, 1> dims1 = {{ 1 }};
            auto dataSpace1 = H5::DataSpace(1, dims1.data());
            auto tidStr = H5::StrType(0, H5T_VARIABLE);
            auto epTags = ep.createAttribute(S_tags, tidStr, dataSpace1);
        }
        
        // general
        {
            writeH5DataSet(ge, S_experiment_description, inParams.m_experimentDescription.c_str());
            writeH5DataSet(ge, S_experimenter, inParams.m_experimenter.c_str());
            writeH5DataSet(ge, S_institution, inParams.m_institution.c_str());
            writeH5DataSet(ge, S_lab, inParams.m_lab.c_str());
            writeH5DataSet(ge, S_session_id, inParams.m_sessionId.c_str());
        }
        
        // stimulus
        auto stPresentation = st.createGroup(S_presentation);
        auto stTemplates = st.createGroup(S_templates);
        
        ac.close();
        an.close();
        ep.close();
        ge.close();
        pr.close();
        st.close();
    }

    void
    writeTopLevelDataSets(H5::H5File & inH5File, MovieNWBExporterParams & inParams)
    {
        auto dataSpace = H5::DataSpace();
        auto tidStr = H5::StrType(0, H5T_VARIABLE);

        auto vs = inH5File.createDataSet(S_nwb_version, tidStr, dataSpace);
        vs.write(std::string(MovieNWBExporterParams::sNwbVersion), tidStr);
        vs.close();

        std::array<hsize_t, 1> dims1 = {{ 1 }};
        auto dataSpace1 = H5::DataSpace(1, dims1.data());
        auto now = isx::Time::now();
        auto fcd = inH5File.createDataSet(S_file_create_date, tidStr, dataSpace1);
        fcd.write(now.getAsIso8601String(), tidStr);
        fcd.close();

        auto id = inH5File.createDataSet(S_identifier, tidStr, dataSpace);
        id.write(inParams.m_identifier, tidStr);
        id.close();

        auto sd = inH5File.createDataSet(S_session_description, tidStr, dataSpace);
        sd.write(inParams.m_sessionDescription, tidStr);
        sd.close();

        auto startTime = inParams.m_srcs[0]->getTimingInfo().getStart();
        auto st = inH5File.createDataSet(S_session_start_time, tidStr, dataSpace);
        st.write(startTime.getAsIso8601String(), tidStr);
        st.close();
    }

    bool
    writeMovieData(H5::Group & inImageSeriesGroup, SpMovie_t & inMovie, double inStartTimeD, AsyncCheckInCB_t & inCheckInCB, isize_t inTotalNumFrames, isize_t & inOutExportedNumFrames)
    {
        auto cancelled = false;
        auto & is = inImageSeriesGroup;
        auto & m = inMovie;

        auto width  = hsize_t(m->getSpacingInfo().getNumColumns());
        auto height = hsize_t(m->getSpacingInfo().getNumRows());
        auto numValidFrames = hsize_t(m->getTimingInfo().getNumValidTimes());
        
        //
        // dataset for video frames ("data")
        //
        std::array<hsize_t, 3> dimsMovie{ {numValidFrames, height, width} };
        auto dataSpaceMovie = H5::DataSpace(3, &dimsMovie[0]);
        const H5::PredType tidPixelMap[] =
            { { H5::PredType::STD_I16LE  },
              { H5::PredType::IEEE_F32LE },
              { H5::PredType::STD_I8LE   } };

        auto tidPixel = tidPixelMap[static_cast<int32_t>(m->getDataType())];
        auto ds = is.createDataSet(S_data, tidPixel, dataSpaceMovie);
        writeH5Attribute(ds, S_conversion, 1.f);
        writeH5Attribute(ds, S_resolution, sqrtf(-1.f));
        writeH5Attribute(ds, S_unit, S_None);

        //
        // dataset for time stamps ("timestamps")
        //
        std::array<hsize_t, 1> dimsTimeStamps{ {numValidFrames} };
        auto dataSpaceTimeStamps = H5::DataSpace(1, &dimsTimeStamps[0]);
        const H5::PredType tidTimeStamp = H5::PredType::IEEE_F64LE;

        auto ts = is.createDataSet(S_timestamps, tidTimeStamp, dataSpaceTimeStamps);
        writeH5Attribute(ts, S_interval, int32_t(1));
        writeH5Attribute(ts, S_unit, S_Seconds);

        //
        // video frame and timestamp data
        //
        auto numSamples = hsize_t(inMovie->getTimingInfo().getNumTimes());
        ISX_ASSERT(numSamples >= numValidFrames);

        hsize_t framesWritten = 0;
        for (hsize_t i = 0; i < numSamples; ++i)
        {
            auto f = inMovie->getFrame(i);
            if (f->getFrameType() == isx::VideoFrame::Type::VALID)
            {
                auto writeIndex = hsize_t(framesWritten);
                // video frame
                {
                    auto fileSpace = ds.getSpace();
                    hsize_t fileStart[3] = {writeIndex, 0, 0};
                    hsize_t fileCount[3] = {1, dimsMovie[1], dimsMovie[2]};
                    fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);

                    H5::DataSpace bufferSpace(3, fileCount);
                    hsize_t bufferStart[3] = { 0, 0, 0 };
                    bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);

                    ds.write((void *) f->getPixels(), tidPixel, bufferSpace, fileSpace);
                }

                // timestamp
                {
                    auto fileSpace = ts.getSpace();
                    hsize_t fileStart[1] = {writeIndex};
                    hsize_t fileCount[1] = {1};
                    fileSpace.selectHyperslab(H5S_SELECT_SET, fileCount, fileStart);
                    
                    H5::DataSpace bufferSpace(1, fileCount);
                    hsize_t bufferStart[1] = { 0 };
                    bufferSpace.selectHyperslab(H5S_SELECT_SET, fileCount, bufferStart);

                    auto timeStamp = f->getTimeStamp().getSecsSinceEpoch().toDouble() - inStartTimeD;
                    ts.write((void *) &timeStamp, tidTimeStamp, bufferSpace, fileSpace);
                }

                ++framesWritten;
            }
            ++inOutExportedNumFrames;
            cancelled = inCheckInCB(float(inOutExportedNumFrames) / float(inTotalNumFrames));
            if (cancelled)
            {
                break;
            }
        }
        ISX_ASSERT(framesWritten == numValidFrames || cancelled);
        
        ds.close();
        ts.close();

        return cancelled;
    }
        
    bool
    writeAnalysisImageSeries(H5::H5File & inH5File, MovieNWBExporterParams & inParams, AsyncCheckInCB_t & inCheckInCB)
    {
        auto cancelled = false;

        auto tidStr = H5::StrType(0, H5T_VARIABLE);

        auto startTime = inParams.m_srcs[0]->getTimingInfo().getStart();
        auto startTimeD = startTime.getSecsSinceEpoch().toDouble();
        
        auto an = inH5File.openGroup(S_analysis);
        
        isize_t totalNumFrames = 0;
        isize_t exportedNumFrames = 0;
        
        for (auto m : inParams.m_srcs)
        {
            totalNumFrames += m->getTimingInfo().getNumTimes();
        }

        for (auto m : inParams.m_srcs)
        {
            auto name = isx::getBaseName(m->getFileName());
            auto is = an.createGroup(name);
            
            {
                std::array<hsize_t, 1> dims2{{2}};
                auto dataSpace2 = H5::DataSpace(1, &dims2[0]);
                auto anc = is.createAttribute(S_ancestry, tidStr, dataSpace2);
                
                const char * ancestryData[] = { S_TimeSeries, S_ImageSeries };
                anc.write(tidStr, (void *)ancestryData);
                anc.close();
            }
            
            writeH5Attribute(is, S_neurodata_type, S_TimeSeries);
            
            // for data source use filename plus project directory name
            // (but don't go further towards root, as it may include the user's name)
            std::string dataSource = m->getFileName();
            auto pos1 = dataSource.rfind('/');
            auto pos2 = dataSource.rfind('/', pos1 - 1);
            auto pos3 = dataSource.rfind('/', pos2 - 1);
            auto pos = std::min(pos1, std::min(pos2, pos3));
            dataSource = dataSource.substr(pos);

            writeH5Attribute(is, S_source, dataSource.c_str());
            int32_t numSamples = int32_t(m->getTimingInfo().getNumValidTimes());
            writeH5DataSet(is, S_num_samples, numSamples);
            int32_t bitsPerPixel = 8 * int32_t(getDataTypeSizeInBytes(m->getDataType()));
            writeH5DataSet(is, S_bits_per_pixel, bitsPerPixel);

            {
                std::array<hsize_t, 1> dims2{{2}};
                auto dataSpace2 = H5::DataSpace(1, &dims2[0]);
                auto tid = H5::PredType::STD_I32LE;
                auto di = is.createDataSet(S_dimension, tid, dataSpace2);
                auto w = int32_t(m->getSpacingInfo().getNumColumns());
                auto h = int32_t(m->getSpacingInfo().getNumRows());
                const int32_t dimensionData[] = { w, h };
                di.write((void *)&dimensionData[0], tid);
                di.close();
            }

            writeH5DataSet(is, S_format, S_raw);
            writeH5Attribute(is, S_comments, inParams.m_comments.c_str());
            writeH5Attribute(is, S_description, inParams.m_description.c_str());

            cancelled = writeMovieData(is, m, startTimeD, inCheckInCB, totalNumFrames, exportedNumFrames);
            if (cancelled)
            {
                break;
            }
        }
        return cancelled;
    }
    
} // namespace

AsyncTaskStatus 
runMovieNWBExporter(MovieNWBExporterParams inParams, std::shared_ptr<MovieNWBExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    bool cancelled = false;
    auto & srcs = inParams.m_srcs;

    // validate inputs
    if (srcs.empty())
    {
        inCheckInCB(1.f);
        return AsyncTaskStatus::COMPLETE;
    }

    for (auto & cs: srcs)
    {
        if (cs == nullptr)
        {
            ISX_THROW(isx::ExceptionUserInput, "One or more of the sources is invalid.");
        }
    }

    if (inParams.m_filename.empty() == false)
    {
        H5::Exception::dontPrint();
        try
        {
            H5::H5File h5File(inParams.m_filename, H5F_ACC_TRUNC);

            writeTopLevelGroups(h5File, inParams);
            writeTopLevelDataSets(h5File, inParams);
            cancelled = writeAnalysisImageSeries(h5File, inParams, inCheckInCB);
            
            h5File.close();
        }
        catch (H5::Exception &e)
        {
            std::remove(inParams.m_filename.c_str());
            ISX_THROW(ExceptionFileIO, e.getDetailMsg() + " " + inParams.m_filename);
        }
    }

    if (cancelled)
    {
        if (!inParams.m_filename.empty())
        {
            std::remove(inParams.m_filename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
