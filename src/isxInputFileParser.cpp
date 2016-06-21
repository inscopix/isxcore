#include "isxInputFileParser.h"
#include "isxHdf5File.h"


namespace isx {

bool 
InputFileParser::isMosaicProject(const std::string & inFileName)
{
    // Figure out if the input is a recording from nVista or a Mosaic Project 
    std::vector<std::string> objInRoot;
    Hdf5File::getObjNames(inFileName, objInRoot);


    bool bMosaicProject = false;
    for (unsigned int obj(0); obj < objInRoot.size(); ++obj)
    {
        if (objInRoot.at(obj) == "MosaicProject")
        {
            bMosaicProject = true;
            break;
        }
    }
    return bMosaicProject;
}


}