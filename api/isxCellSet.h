#ifndef ISX_CELL_SET_H
#define ISX_CELL_SET_H

#include <fstream>
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"



namespace isx
{

class CellSet
{


private:

    /// True if the movie file is valid, false otherwise.
    bool m_valid;

    /// The name of the movie file.
    std::string m_fileName;

    /// The timing information of the movie.
    TimingInfo m_timingInfo;

    /// The spacing information of the movie.
    SpacingInfo m_spacingInfo;

    /// The movie file handle.
    std::fstream m_file;

    /// The header offset.
    std::ios::pos_type m_headerOffset;
};
}
#endif // ISX_CELL_SET_H