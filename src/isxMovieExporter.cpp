#include "isxMovieExporter.h"
#include "isxPathUtils.h"
#include "isxExport.h"
#include "isxException.h"
#include "isxTime.h"
#include "isxMovie.h"

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

namespace isx {

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

AsyncTaskStatus
runMovieExport(MovieExporterParamsWrapper inParams, std::shared_ptr<MovieExporterOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    switch (inParams.m_params->getType())
    {
    case (isx::MovieExporterParams::Type::TIFF):
    {
        auto params = *(isx::MovieTiffExporterParams*)inParams.m_params.get();
        auto outparams = std::static_pointer_cast<MovieTiffExporterOutputParams>(inOutputParams);
        return runMovieTiffExporter(params, outparams, inCheckInCB);
    }
    break;
    case (isx::MovieExporterParams::Type::NWB):
    {
        auto params = *(isx::MovieNWBExporterParams*)inParams.m_params.get();
        auto outparams = std::static_pointer_cast<MovieNWBExporterOutputParams>(inOutputParams);
        return runMovieNWBExporter(params, outparams, inCheckInCB);
    }
    break;
    case (isx::MovieExporterParams::Type::MP4):
    {
        auto params = *(isx::MovieCompressedAviExporterParams*)inParams.m_params.get();
        auto outparams = std::static_pointer_cast<MovieCompressedAviExporterOutputParams>(inOutputParams);
        return runMovieCompressedAviExporter(params, outparams, inCheckInCB);
    }
    break;
    default:
        break;
    }

    inCheckInCB(1.f);
    return AsyncTaskStatus::COMPLETE;
}

} // namespace isx
