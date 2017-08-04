#include "isxGpioImporter.h"
#include "isxNVokeGpioFile.h"
#include "isxNVistaGpioFile.h"
#include "isxPathUtils.h"

namespace isx
{

std::string
GpioDataParams::getOpName()
{
    return "GPIO Import";
}

AsyncTaskStatus runGpioDataImporter(GpioDataParams inParams, std::shared_ptr<GpioDataOutputParams> inOutputParams, AsyncCheckInCB_t inCheckInCB)
{
    std::string extension = isx::getExtension(inParams.fileName);
    isx::AsyncTaskStatus result = isx::AsyncTaskStatus::COMPLETE;

    if (extension == "raw")
    {
        NVokeGpioFile raw(inParams.fileName, inParams.outputDir);
        raw.setCheckInCallback(inCheckInCB);

        result = raw.parse();
    
        raw.getOutputFileNames(inOutputParams->filenames);
    }
    else if (extension == "hdf5")
    {
        NVistaGpioFile input(inParams.fileName, inParams.outputDir, inCheckInCB);
        result = input.parse();
        input.getOutputFileNames(inOutputParams->filenames);
    }
    

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