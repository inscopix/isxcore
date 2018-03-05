#include "isxBehavMovieImporter.h"
#include "isxAsyncTaskHandle.h"
#include "isxBehavMovieFile.h"

#include "json.hpp"

namespace isx
{
    
std::string
BehavMovieImportParams::getOpName()
{
    return "Behavioral movie import";
}

std::string
BehavMovieImportParams::toString() const
{
    using json = nlohmann::json;
    json j;
    j["fileName"] = fileName;
    return j.dump(4);
}

AsyncTaskStatus runBehavMovieImporter(
    BehavMovieImportParams inParams, 
    std::shared_ptr<BehavMovieImportOutputParams> inOutputParams, 
    AsyncCheckInCB_t inCheckInCB)
{
    if (BehavMovieFile::getBehavMovieProperties(inParams.fileName, inOutputParams->m_dataSetProperties, inCheckInCB))
    {
        return AsyncTaskStatus::COMPLETE;
    }

    return AsyncTaskStatus::CANCELLED;
}
    
} // namespace isx
