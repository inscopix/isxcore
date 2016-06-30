#include "isxHdf5File.h"
//#include "isxLog.h"
#include "isxException.h"
#include "H5Cpp.h"

namespace isx
{
void
Hdf5File::getObjNames(const std::string & inFileName, const std::string & inPath, std::vector<std::string> & outNames)
{
    H5::Group rootGroup;

    try
    {
        H5::H5File file(inFileName.c_str(), H5F_ACC_RDONLY);
        rootGroup = file.openGroup(inPath);
    }
    catch (const H5::FileIException& error)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failure caused by H5File operations.\n", error.getDetailMsg());
    }


    hsize_t nObjInGroup = rootGroup.getNumObjs();

    if (0 == nObjInGroup)
    {
        return;
    }

    outNames.resize((size_t)nObjInGroup);
    for (unsigned int i(0); i < nObjInGroup; ++i)
    {
        outNames[i] = rootGroup.getObjnameByIdx(i);
    }

}

} // namespace isx