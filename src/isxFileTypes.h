#ifndef ISX_FILE_TYPES_H
#define ISX_FILE_TYPES_H

#include <string>

namespace isx
{
    

    enum class FileType 
    {
        V1,     ///< V1 of GPIO file format (legacy) 
        V2      ///< V2 of GPIO/EVENTS file format, contains both dense and sparse signals
    };

    FileType getFileType(const std::string & inFileName);
}

#endif /// ISX_FILE_TYPES_H