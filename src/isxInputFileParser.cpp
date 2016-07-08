#include "isxInputFileParser.h"
#include "isxHdf5Utils.h"
#include "isxProjectFile.h"


namespace isx {

bool 
InputFileParser::isMosaicProject(const std::string & inFileName)
{
    // Figure out if the input is a recording from nVista or a Mosaic Project 
    std::vector<std::string> objInRoot;
    isx::internal::getHdf5ObjNames(inFileName, "/", objInRoot);

    bool bMosaicProject = false;
    for (unsigned int obj(0); obj < objInRoot.size(); ++obj)
    {
        if (objInRoot.at(obj) == "MosaicProject")
        {
            std::vector<std::string> objInMosaicProject;
            isx::internal::getHdf5ObjNames(inFileName, "/MosaicProject", objInMosaicProject);

            int counter = 0;
            for (size_t i(0); i < objInMosaicProject.size(); ++i)
            {
                std::string objName = "/MosaicProject/" + objInMosaicProject[i];
                if ((objName == ProjectFile::headerPath) ||
                    (objName == ProjectFile::seriesPath) ||
                    (objName == ProjectFile::historyPath) ||
                    (objName == ProjectFile::annotationsPath) ||
                    (objName == ProjectFile::cellsPath))
                {
                    ++counter;
                }
            }

            if (counter == 5)
            {
                bMosaicProject = true;
            }

            break;
        }
    }
    return bMosaicProject;
}

std::string 
InputFileParser::getExtension(const std::string & inFileName)
{
    return inFileName.substr(inFileName.find_last_of(".") + 1);
}


}
