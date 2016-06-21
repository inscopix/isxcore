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
        /// \param outNames a vector of objects under the hdf5 root
        ///
        static void getObjNames(const std::string & inFileName, std::vector<std::string> & outNames);
    };
}

#endif // def ISX_HDF5_FILE_H
