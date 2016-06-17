#ifndef ISX_PROJECTFILE_H
#define ISX_PROJECTFILE_H

// This file describes the file specification for a Mosaic project file.
// The project file is a HDF5 file with the following data model:
/*

/MosaicProject 
    /FileHeader
        /Mosaic Version
        /Operating System
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
        /// constructor
        ///
        ProjectFile(const std::string & inFileName);
        
        /// destructor
        ///
        ~ProjectFile();

        /// Get the file handle
        ///
        SpHdf5FileHandle_t getHdf5FileHandle();

        static const std::string projectPath;
        static const std::string headerPath;
        static const std::string seriesPath;
        
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
