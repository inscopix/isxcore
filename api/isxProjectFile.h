#ifndef ISX_PROJECTFILE_H
#define ISX_PROJECTFILE_H

// This file describes the file specification for a Mosaic project file.
// The project file is a HDF5 file with the following data model:
/*

/MosaicProject 
    /FileHeader
        /Mosaic Version
        /Operating System
        /Original Input File
    /History
        /History item 1
            /Type
            /Parameters
            /InputMovies         ---> to be saved only for the last history item (if needed to undo an operation)
                /Series item 1
                    /Movie 1
                        :
                        :
                    /Movie L
                    :
                    :
                /Series item P
            :
            :
        /History item N
    /Annotations
        /Comment 1
            /Text
            /Timestamp
            :
            :
        /Comment M
    /Series
        /Series item 1
            /Recording item 1
                /Properties
                    /FrameRate
                    /DateOfCreation
                /Movie
                :
                :
            /Recording item L
            :
            :
        /Series item P
    /Cells
        /Cell Set 1
            /Cell set ID
            /Cells
                /Cell 1
                    /Properties
                        /isValid
                        /Cell_ID
                        /Comment
                    /Contour
                    /Trace
                    :
                    :
                /Cell K
            :
            :
        /Cell Set J
        
*/
#include "isxTime.h"
#include "isxCoreFwd.h"
#include "isxMovieSeries.h"
#include <utility>
#include <vector>
#include <memory>

namespace isx {

    
    /// The project file class
    ///
    class ProjectFile
    {
    public:

        /// constructor - valid c++ object but invalid file
        ProjectFile();

        /// constructor - open existing file
        /// \param inFileName the file to open
        ProjectFile(const std::string & inFileName);

        /// constructor - create new file
        /// \param inFileName the name of the project file to create
        /// \param inInputFileName the name of the original data set used to initialize this file
        ProjectFile(const std::string & inFileName, const std::string & inInputFileName);
        
        /// destructor
        ///
        ~ProjectFile();

        /// Get the file handle
        ///
        SpHdf5FileHandle_t getHdf5FileHandle();
        
        /// \return whether the file is valid or not
        ///
        bool isValid();

        /// the path for the mosaic project in the HDF5 file
        ///
        static const std::string projectPath;
        
        /// the path for the file header in the HDF5 file
        ///
        static const std::string headerPath;
        
        /// the path for the movie series in the HDF5 file
        ///
        static const std::string seriesPath;

        /// the path for the history in the HDF5 file
        ///
        static const std::string historyPath;

        /// the path for the annotations in the HDF5 file
        ///
        static const std::string annotationsPath;

        /// the path for the cell traces in the HDF5 file
        ///
        static const std::string cellsPath;
        
        /// Get the number of recording series in the project file
        ///
        uint16_t getNumMovieSeries();
        
        /// Get a recording series by index
        ///
        SpMovieSeries_t getMovieSeries(uint16_t inIndex);
        
        /// Add a recording series to the project file
        ///
        SpMovieSeries_t addMovieSeries(const std::string & inName);
       
    private:        
        class Impl;
        /// Internal implementation of ProjectFile class
        ///
        std::unique_ptr<Impl> m_pImpl;
        
    };
 
}
#endif // ISX_PROJECTFILE_H
