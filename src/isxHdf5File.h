#ifndef ISX_HDF5_FILE_H
#define ISX_HDF5_FILE_H


#include <vector>
#include <string>

namespace isx
{

    /// wrapper for HDF5 file providing basic generic functionality to query the contents of the file
    class Hdf5File
    {
    public:

        /// Opens the file for read only and gets the names of the groups under the root
        /// \param inFileName the file to open
        /// \param inPath the path to check
        /// \param outNames a vector of objects under the path inPath
        ///
        static void getObjNames(const std::string & inFileName, const std::string & inPath, std::vector<std::string> & outNames);
    };
}

#endif // def ISX_HDF5_FILE_H
