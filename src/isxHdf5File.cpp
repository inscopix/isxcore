#include "isxHdf5File.h"
#include "H5Cpp.h"

namespace isx
{
void
Hdf5File::getObjNames(const std::string & inFileName, std::vector<std::string> & outNames)
{
    H5::H5File file(inFileName.c_str(), H5F_ACC_RDONLY);
    std::string rootObjName("/");
    H5::Group rootGroup = file.openGroup(rootObjName);
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