#include "isxMovieTiffExporter.h"
#include "isxExport.h"
#include "isxException.h"
#include "isxMovie.h"

#include <vector>

#include "json.hpp"

namespace isx {

std::string
MovieTiffExporterParams::getOpName()
{
    return "Export TIFF";
}

std::string
MovieTiffExporterParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["filename"] = m_filename;
    j["writeInvalidFrames"] = m_writeInvalidFrames;
    j["numFramesInMovie"] = m_numFramesInMovie;
    return j.dump(4);
}

MovieExporterParams::Type
MovieTiffExporterParams::getType()
{
    return MovieExporterParams::Type::TIFF;
}

void
MovieTiffExporterParams::setOutputFileName(const std::string & inFileName)
{
    m_filename = inFileName;
}

void
MovieTiffExporterParams::setWirteDroppedAndCroppedParameter(const bool inWriteDroppedAndCropped)
{
    m_writeInvalidFrames = inWriteDroppedAndCropped;
}

void
MovieTiffExporterParams::setSources(const std::vector<SpMovie_t> & inSources)
{
    m_srcs = inSources;
}

void
MovieTiffExporterParams::setAdditionalInfo(
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
    // Do nothing - TIFF cannot contains these details
}

std::vector<std::string>
MovieTiffExporterParams::getInputFilePaths() const
{
    std::vector<std::string> inputFilePaths;
    for (const auto & s : m_srcs)
    {
        inputFilePaths.push_back(s->getFileName());
    }
    return inputFilePaths;
}

std::vector<std::string>
MovieTiffExporterParams::getOutputFilePaths() const
{
    return {m_filename};
}

AsyncTaskStatus 
runMovieTiffExporter(MovieTiffExporterParams inParams, std::shared_ptr<MovieTiffExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
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
        try
        {
            // If there is only one source with one frame, then write a regular TIFF image.
            // Otherwise, write a BigTIFF movie.
            if ((inParams.m_srcs.size() == 1) && (inParams.m_srcs.front()->getTimingInfo().getNumTimes() == 1))
            {
                const isx::SpVideoFrame_t frame = inParams.m_srcs.front()->getFrame(0);
                toTiff(inParams.m_filename, &frame->getImage());
            }
            else
            {
                cancelled = toTiff(inParams.m_filename, inParams.m_srcs, inParams.m_writeInvalidFrames, inParams.m_numFramesInMovie, inCheckInCB);
            }
        }
        catch (...)
        {
            std::remove(inParams.m_filename.c_str());
            throw;
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
