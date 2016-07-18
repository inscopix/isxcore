#include "isxInputFileParser.h"
#include "isxHdf5Utils.h"

namespace isx {

bool 
InputFileParser::isMosaicProject(const std::string & inFileName)
{   
    std::string extension = getExtension(inFileName);
    
    if (extension == "isxp")
    {
        return true;
    }

    return false;
}

std::string 
InputFileParser::getExtension(const std::string & inFileName)
{
    return inFileName.substr(inFileName.find_last_of(".") + 1);
}


}
