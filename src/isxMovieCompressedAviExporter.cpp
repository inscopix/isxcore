#include "isxMovieCompressedAviExporter.h"
#include "isxExportTiff.h"
#include "isxExportCompressedAVI.h"
#include "isxException.h"

#include <vector>

namespace isx {

std::string
MovieCompressedAviExporterParams::getOpName()
{
    return "Export Avi Movie";
}

AsyncTaskStatus 
runMovieCompressedAviExporter(MovieCompressedAviExporterParams inParams, std::shared_ptr<MovieCompressedAviExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
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


    if (inParams.m_compressedAviFilename.empty() == false)
    {
        try
        {
            cancelled = toCompressedAVI(inParams.m_compressedAviFilename, inParams.m_srcs, inCheckInCB);
        }
        catch (...)
        {
            std::remove(inParams.m_compressedAviFilename.c_str());
            throw;
        }
    }

    if (cancelled)
    {
        if (!inParams.m_compressedAviFilename.empty())
        {
            std::remove(inParams.m_compressedAviFilename.c_str());
        }

        return AsyncTaskStatus::CANCELLED;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
