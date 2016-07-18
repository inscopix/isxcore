#ifndef ISX_PROJECTFILE_H
#define ISX_PROJECTFILE_H

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

        enum DataFileType
        {
            PF_DATAFILETYPE_MOVIE = 0,
            PF_DATAFILETYPE_CELLSET,
            PF_DATAFILETYPE_IMAGE,
            PF_DATAFILETYPE_TIMESERIES
        };

        struct DataFileDescriptor
        {
            DataFileType type;
            std::string  filename;
        };

        struct DataCollection
        {
            std::string name;
            std::vector<DataFileDescriptor> files;
        };

        /// constructor - valid c++ object but invalid file
        ProjectFile();

        /// constructor - open existing file
        /// \param inFileName the file to open
        ProjectFile(const std::string & inFileName);
        
        /// destructor
        ///
        ~ProjectFile();

        /// \return whether the file is valid or not
        ///
        bool isValid();

        /// Get the number of file collections in the project file
        ///
        isize_t getNumDataCollections();

        /// Get the filenames of all the files within a file collection
        ///
        DataCollection getDataCollection(isize_t inIndex);

        /// Add a file collection to the project by passing the filenames of all the files within a collection
        ///
        void addDataCollection(DataCollection & inData);

        /// Remove data collection from project
        ///
        void removeDataCollection(isize_t inCollectionIndex);

        /// Append a file to a data collection
        ///
        void addFileToDataCollection(DataFileDescriptor & inFileDesc, isize_t inCollectionIndex);

        /// Remove a file from a data collection
        ///
        void removeFileFromDataCollection(isize_t inFileDescIndex, isize_t inCollectionIndex);

        /// Get file name
        ///
        std::string getName();

        /// Get filenames for the original files (from where the data was obtained)
        ///
        std::vector<std::string> & getOriginalNames();
        
       
    private:        
        class Impl;
        /// Internal implementation of ProjectFile class
        ///
        std::unique_ptr<Impl> m_pImpl;
        
    };
 
}
#endif // ISX_PROJECTFILE_H
