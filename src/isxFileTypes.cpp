#include "isxFileTypes.h"
#include "isxJsonUtils.h"
#include <fstream>

namespace isx 
{

FileType getFileType(const std::string & inFileName)
{
    FileType type = FileType::V1;
    std::fstream file(inFileName, std::ios::binary | std::ios_base::in);
    if (!file.good() || !file.is_open())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open file: ", inFileName);
    }

    std::ios::pos_type headerOffset;
    
    try
    {
        json j = readJsonHeaderAtEnd(file, headerOffset);
        if ((j.find("fileType") != j.end()))
        {
            type = FileType(j.at("fileType").get<int>());
        }       
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO, "Error parsing header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing header.");
    }
    return type;
}

}