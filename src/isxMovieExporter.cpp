#include "isxMovieExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxException.h"
#include "isxTime.h"
#include "isxMovie.h"
#include "isxSeriesUtils.h"

#include "isxMovieNWBExporter.h"
#include "isxMovieTiffExporter.h"
#include "isxMovieCompressedAviExporter.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <array>
#include <cmath>
#include <memory>
#include <json.hpp>

namespace isx {

/// MovieTimestampExporterParams

void 
MovieTimestampExporterParams::setSources(const std::vector<SpMovie_t> & inSources)
{
    m_srcs = inSources;
}

std::string
MovieTimestampExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["timestampCsvFilename"] = m_outputFilename;

    std::string timestampFormat;
    if (m_format == WriteTimeRelativeTo::TSC)
    {
        timestampFormat = "TSC";
    }
    else if (m_format == WriteTimeRelativeTo::FIRST_DATA_ITEM)
    {
        timestampFormat = "Relative to movie start";
    }
    else
    {
        timestampFormat = "Unix";
    }
    j["timestampFormat"] = timestampFormat;
    return j.dump(4);
}

std::vector<std::string>
MovieTimestampExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
MovieTimestampExporterParams::getOutputFilePaths() const
{
    return {m_outputFilename};
}

/// MovieExporterParamsWrapper

std::string
MovieExporterParamsWrapper::getOpName()
{
    return "Export Movie";
}

std::string
MovieExporterParamsWrapper::toString() const
{
    using json = nlohmann::json;
    json j;

    if (m_params)
    {
        j = json::parse(m_params->toString());
    }

    if (m_timestampParams)
    {
        j += json::parse(m_timestampParams->toString());
    }

    return j.dump(4);
}

void
MovieExporterParamsWrapper::setOutputFileName(const std::string & inFileName)
{
    if (m_params)
    {
        m_params->setOutputFileName(inFileName);
    }
}

void 
MovieExporterParamsWrapper::setSources(const std::vector<SpMovie_t> & inSources)
{
    if (m_params)
    {
        m_params->setSources(inSources);
    }

    if (m_timestampParams)
    {
        m_timestampParams->setSources(inSources);
    }
}

void
MovieExporterParamsWrapper::setWriteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped)
{
    if (m_params)
    {
        m_params->setWriteDroppedAndCroppedParameter(inWriteDroppedAndCropped);
    }
}

void
MovieExporterParamsWrapper::setBitRateFraction(const double inBitRateFraction)
{
    if (m_params)
    {
        m_params->setBitRateFraction(inBitRateFraction);
    }
}

void
MovieExporterParamsWrapper::setAdditionalInfo(
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
    if (m_params)
    {
        m_params->setAdditionalInfo(
            inIdentifierBase, 
            inSessionDescription, 
            inComments, 
            inDescription, 
            inExperimentDescription, 
            inExperimenter,
            inInstitution,
            inLab, 
            inSessionId);
    }
}

std::vector<std::string>
MovieExporterParamsWrapper::getInputFilePaths() const
{
    if (m_params)
    {
        return m_params->getInputFilePaths();
    }
    else
    {
        ISX_ASSERT(m_timestampParams);
        return m_timestampParams->getInputFilePaths();
    }
}

std::vector<std::string>
MovieExporterParamsWrapper::getOutputFilePaths() const
{
    std::vector<std::string> outputFilePaths;

    if (m_params)
    {
        outputFilePaths = m_params->getOutputFilePaths();
    }

    if (m_timestampParams)
    {
        const auto tsOutputFilePaths = m_timestampParams->getOutputFilePaths();
        outputFilePaths.insert(outputFilePaths.end(), tsOutputFilePaths.begin(), tsOutputFilePaths.end());
    }

    return outputFilePaths;
}

MovieExporterParamsWrapper 
makeMovieExporterParamsWrapper(MovieExporterParams::Type inType)
{
    isx::MovieExporterParamsWrapper output;

    switch (inType)
    {
        case (MovieExporterParams::Type::NWB):
            output.m_params = std::make_shared<isx::MovieNWBExporterParams>();
            break;

        case (MovieExporterParams::Type::TIFF):
            output.m_params = std::make_shared<isx::MovieTiffExporterParams>();
            break;

        case (MovieExporterParams::Type::MP4):
            output.m_params = std::make_shared<isx::MovieCompressedAviExporterParams>();
            break;
    }

    return output;
}

/// Helper functions

// Returns the first tsc value of a movie
// If the first frame of the movie is dropped, the tsc value is interpolated
// using the tsc value of the first frame and the average sampling rate of the movie.
uint64_t getFirstTsc(
    const SpMovie_t & inMovie
)
{
    ISX_ASSERT(inMovie->hasFrameTimestamps());

    // Get first TSC of the inMovie
    if (inMovie->getTimingInfo().isIndexValid(0))
    {
        return inMovie->getFrameTimestamp(0);
    }
    else
    {
        // Handle case where there is no tsc value for the first frame of the inMovie
        const auto & ti = inMovie->getTimingInfo();
        size_t firstValidIdx = 0;
        for (size_t i = 1; i < ti.getNumTimes(); i++)
        {
            if (ti.isIndexValid(i))
            {
                firstValidIdx = i;
                break;
            }
        }

        if (firstValidIdx == 0)
        {
            ISX_THROW(isx::Exception, "Failed to find index of first valid frame in isxb file.");
        }

        const auto firstValidTsc = inMovie->getFrameTimestamp(firstValidIdx);
        const auto stepSizeUs = ti.getStep().toDouble() * 1e6;
        return static_cast<uint64_t>(std::round(double(firstValidTsc) - stepSizeUs * double(firstValidIdx)));
    }
}

/// Export functions

AsyncTaskStatus
runMovieTimestampExport(const MovieTimestampExporterParams inParams, AsyncCheckInCB_t inCheckInCB, const float inProgressAllocation, const float inProgressStart)
{
    // Ensure input movie series is ordered in time
    {
        const auto & movies = inParams.m_srcs;
        std::string errorMessage;
        for (isize_t i = 1; i < movies.size(); ++i)
        {
            if (!checkNewMemberOfSeries({movies[i - 1]}, movies[i], errorMessage))
            {
                ISX_THROW(ExceptionSeries, errorMessage);
            }

            if (movies[i]->getTimingInfo().getStart() < movies[i - 1]->getTimingInfo().getStart())
            {
                ISX_THROW(ExceptionSeries, "Members of series are not ordered in time.");
            }

        }
    }

    size_t totalNumFrames = 0;
    for (const auto & movie : inParams.m_srcs)
    {
        if (!movie->hasFrameTimestamps())
        {
            ISX_THROW(isx::ExceptionUserInput, "Input movie does not have frame timestamps stored in file.");
        }

        totalNumFrames += movie->getTimingInfo().getNumTimes();
    }

    std::ofstream csv(inParams.m_outputFilename);
    csv << "Global Frame Number,Movie Number,Local Frame Number,Frame Timestamp ";
    if (inParams.m_format == WriteTimeRelativeTo::TSC)
    {
        csv << "(us)";
    }
    else
    {
        csv << "(s)";
    }
    csv << std::endl;

    size_t globalFrameNumber = 0;
    const auto firstTsc = getFirstTsc(inParams.m_srcs.front());
    const double epochStartTimestamp = double(inParams.m_srcs.front()->getTimingInfo().getStart().getSecsSinceEpoch().getNum()) / 1e3;
    for (size_t movieNumber = 0; movieNumber < inParams.m_srcs.size(); movieNumber++)
    {
        const auto & movie = inParams.m_srcs[movieNumber];
        const auto & timingInfo = movie->getTimingInfo();
        for (size_t localFrameNumber = 0; localFrameNumber < timingInfo.getNumTimes(); localFrameNumber++)
        {
            csv << globalFrameNumber << "," << movieNumber << "," << localFrameNumber << ",";
            if (timingInfo.isIndexValid(localFrameNumber))
            {
                const auto tsc = movie->getFrameTimestamp(localFrameNumber);
                if (inParams.m_format == WriteTimeRelativeTo::FIRST_DATA_ITEM)
                {
                    const auto timestamp = double(tsc - firstTsc) / 1e6;
                    csv << std::fixed << std::setprecision(6) << timestamp;
                }
                else if (inParams.m_format == WriteTimeRelativeTo::UNIX_EPOCH)
                {
                    const auto timestamp = epochStartTimestamp + (double(tsc - firstTsc) / 1e6);
                    csv << std::fixed << std::setprecision(6) << timestamp;
                }
                else
                {
                    csv << tsc;
                }
            }
            else
            {
                csv << "NaN";
            }
            csv << std::endl;
            if (inCheckInCB(inProgressStart + (float(++globalFrameNumber) / float(totalNumFrames)) * inProgressAllocation))
            {
                return AsyncTaskStatus::CANCELLED;
            }
        }
        
    }

    return AsyncTaskStatus::COMPLETE;
}

AsyncTaskStatus
runMovieExport(MovieExporterParamsWrapper inParams, std::shared_ptr<MovieExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    float progressAllocation = 1.0f;
    float progressStart = 0.0f;

    AsyncTaskStatus status = AsyncTaskStatus::PENDING;
    if (inParams.m_timestampParams)
    {
        // If exporting frames and timestamps then allocate 5% of this operation to exporting timestamps
        if (inParams.m_params)
        {
            progressAllocation = 0.05f;
        }
        status = runMovieTimestampExport(*inParams.m_timestampParams, inCheckInCB, progressAllocation, progressStart);

        if (status == AsyncTaskStatus::CANCELLED)
        {
            return status;
        }

        if (inParams.m_params)
        {
            progressStart += progressAllocation;
            progressAllocation = 1.0f - progressStart;
        }
    }

    if (inParams.m_params)
    {
        switch (inParams.m_params->getType())
        {
        case (isx::MovieExporterParams::Type::TIFF):
        {
            auto params = *(isx::MovieTiffExporterParams*)inParams.m_params.get();
            auto outparams = std::static_pointer_cast<MovieTiffExporterOutputParams>(inOutputParams);
            status = runMovieTiffExporter(params, outparams, inCheckInCB, progressAllocation, progressStart);
        }
        break;
        case (isx::MovieExporterParams::Type::NWB):
        {
            auto params = *(isx::MovieNWBExporterParams*)inParams.m_params.get();
            auto outparams = std::static_pointer_cast<MovieNWBExporterOutputParams>(inOutputParams);
            status = runMovieNWBExporter(params, outparams, inCheckInCB, progressAllocation, progressStart);
        }
        break;
        case (isx::MovieExporterParams::Type::MP4):
        {
            auto params = *(isx::MovieCompressedAviExporterParams*)inParams.m_params.get();
            auto outparams = std::static_pointer_cast<MovieCompressedAviExporterOutputParams>(inOutputParams);
            status = runMovieCompressedAviExporter(params, outparams, inCheckInCB, progressAllocation, progressStart);
        }
        break;
        default:
            break;
        }
    }

    inCheckInCB(1.f);
    return status;
}

} // namespace isx
