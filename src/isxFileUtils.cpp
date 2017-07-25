#include "isxFileUtils.h"
#include "isxHdf5Utils.h"
#include <algorithm>

namespace isx
{

Hdf5Mode peekHdf5Modality(const std::string & inFileName)
{
    std::vector<std::string> objects;
    internal::getHdf5ObjNames(inFileName, "/", objects);

    Hdf5Mode mode = Hdf5Mode::NONE;
    auto it = std::find(objects.begin(), objects.end(), "anc_data");
    if (it != objects.end())
    {
        mode |= Hdf5Mode::GPIO;
    }

    it = std::find(objects.begin(), objects.end(), "images");
    if (it != objects.end())
    {
        mode |= Hdf5Mode::IMAGING;
    }

    return mode;
}

}