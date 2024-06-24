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

std::vector<std::string>
BehavMovieImportParams::getInputFilePaths() const
{
    return {fileName};
}

std::vector<std::string>
BehavMovieImportParams::getOutputFilePaths() const
{
    return {};
}

} // namespace isx
