#include "isxGpioImporter.h"
#include "isxNVokeGpioFile.h"

namespace isx
{

std::string
GpioDataParams::getOpName()
{
    return "GPIO Import";
}

AsyncTaskStatus runGpioDataImporter(GpioDataParams inParams, std::shared_ptr<GpioDataOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{

    NVokeGpioFile raw(inParams.fileName, inParams.outputDir);
    raw.setCheckInCallback(inCheckInCB);

    isx::AsyncTaskStatus result = raw.parse();
    
    raw.getOutputFileNames(inOutputParams->filenames);

    if (result == isx::AsyncTaskStatus::CANCELLED)
    {
        for(auto & fn : inOutputParams->filenames)
        {
            std::remove(fn.c_str());
        }
    }
    
    return result;
}

}