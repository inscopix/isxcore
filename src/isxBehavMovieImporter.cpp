#include "isxBehavMovieImporter.h"
#include "isxAsyncTaskHandle.h"
#include "isxBehavMovieFile.h"

namespace isx
{
    
std::string
BehavMovieImportParams::getOpName()
{
    return "Behavioral movie import";
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
