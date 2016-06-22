#ifndef ISX_INPUTFILEPARSER_H
#define ISX_INPUTFILEPARSER_H

#include <string>


namespace isx {
    /// A class to parse the input files 
    ///
    class InputFileParser
    {
    public: 

        /// Query whether the file described by the filename is a mosaic project or a recording file
        /// \param inFileName - file name of interest
        static bool isMosaicProject(const std::string & inFileName);
       

    };
}

#endif // ISX_INPUTFILEPARSER_H