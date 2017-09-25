#include "isxMovieTiffExporter.h"
//#include "isxExport.h"
#include "isxExportTiff.h"
#include "isxException.h"

#include <vector>

namespace isx {

std::string
MovieTiffExporterParams::getOpName()
{
    return "Export Tiff Movie";
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


    if (inParams.m_tiffFilename.empty() == false)
    {
        try
        {
            toTiff(inParams.m_tiffFilename, inParams.m_srcs, inParams.m_numFramesInMovie);
        }
        catch (...)
        {
            std::remove(inParams.m_tiffFilename.c_str());
            throw;
        }
    }

    if (cancelled)
    {
        if (!inParams.m_tiffFilename.empty())
        {
            std::remove(inParams.m_tiffFilename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
