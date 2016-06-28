#include "isxHdf5Utils.h"
#include "H5Cpp.h"

namespace isx
{

namespace internal
{

void getHdf5ObjNames(
    const std::string & inFileName,
    const std::string & inPath,
    std::vector<std::string> & outNames)
{
    H5::H5File file(inFileName.c_str(), H5F_ACC_RDONLY);
    H5::Group rootGroup = file.openGroup(inPath);
    hsize_t nObjInGroup = rootGroup.getNumObjs();

    if (0 == nObjInGroup)
    {
        return;
    }

    outNames.resize(nObjInGroup);
    for (size_t i(0); i < nObjInGroup; ++i)
    {
        outNames[i] = rootGroup.getObjnameByIdx(i);
    }
}

} // namespace internal

} // namespace isx
