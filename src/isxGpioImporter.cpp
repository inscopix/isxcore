#include "isxGpioImporter.h"
#include "isxNVokeGpioFile.h"
#include "isxNVistaGpioFile.h"
#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"

#include "json.hpp"

namespace isx
{

std::string
GpioDataParams::getOpName()
{
    return "GPIO Import";
}

std::string
GpioDataParams::toString() const
{
    using json = nlohmann::json;
    json j;
    return j.dump(4);
}

std::vector<std::string>
GpioDataParams::getInputFilePaths() const
{
    return {fileName};
}

std::vector<std::string>
GpioDataParams::getOutputFilePaths() const
{
    // We do not know the output file paths, but at least we know what directory
    // they will be in.
    return {outputDir};
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
        inOutputParams->filenames = {raw.getOutputFileName()};
    }
    else if (extension == "hdf5")
    {
        NVistaGpioFile input(inParams.fileName, inParams.outputDir, inCheckInCB);
        result = input.parse();
        inOutputParams->filenames = {input.getOutputFileName()};
    }
    else if (extension == "dump")
    {
        NVista3GpioFile dump(inParams.fileName, inParams.outputDir);
        dump.setCheckInCallback(inCheckInCB);
        result = dump.parse();
        inOutputParams->filenames = {dump.getOutputFileName()};
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

} // namespace
