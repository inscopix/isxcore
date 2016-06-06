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
                /Schedule item 1
                    /Movie 1
                        :
                        :
                    /Movie L
                    :
                    :
                /Schedule item P
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
    /Schedules
        /Schedule item 1
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
        /Schedule item P
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

#include <utility>
#include <vector>
#include <memory>


#pragma pack(push, 1)
namespace isx {
    
    const int COMMENT_LENGTH = 256;

    /// enum for the available types for history records
    ///
    enum HistoryItemType
    {
        HISTORYITEMTYPE_SPATIAL_CROP = 0,
        HISTORYITEMTYPE_TEMPORAL_CROP,
        HISTORYITEMTYPE_SPATIAL_DOWNSAMPLE,
        HISTORYITEMTYPE_TEMPORAL_DOWNSAMPLE,
        HISTORYITEMTYPE_F_NORM,
        HISTORYITEMTYPE_CELL_SEG
    };

    ///TODO: Define parameters for each of the operations defined in History

    /// The annotation item basic structure
    ///
    struct FileComment
    {
        uint64_t nTimestamp;              //!< Frame index associated to the comment 
        char     cText[COMMENT_LENGTH];   //!< Comment text 
    };

    /// Video properties basic structure
    ///
    struct VideoProperties
    {
        double   dFrameRate;             //!< The frame rate for the video 
        Time     dateOfCreation;         //!< The date of creation 
    };

    /// Cell properties basic structure
    ///
    struct CellProperties
    {
        uint64_t cellId;                //!< Unique cell ID for each cell identified in the video 
        uint64_t cText[COMMENT_LENGTH]; //!< A generic comment to attach to each cell 
    };

    /// A vector of <Row, Column> pairs describing the contour of an identified cell
    ///
    typedef std::vector<std::pair<uint16_t, uint16_t> > CellContour;    


    
    /// The project file class
    ///
    class ProjectFile
    {
    public:
        /// constructor
        ///
        ProjectFile(std::string & inFileName);
        
        /// destructor
        ///
        ~ProjectFile();

        /// Get the file handle
        ///
        SpHdf5FileHandle_t getHdf5FileHandle();
       
    private:        
        class Impl;
        /// Internal implementation of ProjectFile class
        ///
        std::unique_ptr<Impl> m_pImpl;
        
    };
 
}
#pragma pack(pop)
#endif // ISX_PROJECTFILE_H
