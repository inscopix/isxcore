#ifndef ISX_PROJECT_H
#define ISX_PROJECT_H

#include "isxWritableMovie.h"
#include "isxProjectFile.h"
#include "isxCoreFwd.h"

#include <utility>
#include <vector>
#include <memory>

namespace isx
{

/// The project model, through which all data read/write should occur.
///
class Project
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid c++ object but invalid project.
    Project();

    /// Constructor.
    ///
    /// If the file exists, this reads that project.
    /// If not, this creates a new project with a root data collection.
    ///
    /// \param  inFileName  The file name of the project.
    Project(const std::string & inFileName);

    /// \return     True if the project is valid, false otherwise.
    ///
    bool isValid();

    /// Create a movie in the root data collection of this project.
    ///
    /// \param  inFileName      The name of the file to store the new movie.
    /// \param  inTimingInfo    The timing info of the movie to create.
    /// \param  inSpacingInfo   The spacing info of the movie to create.
    /// \return                 A shared pointer to the created movie.
    ///
    /// \throw  isx::ExceptionFileIO    If the file already exists.
    /// \throw  isx::ExceptionDataIO    If formatting the movie file fails.
    SpWritableMovie_t createMosaicMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo);

    /// Get a data collection by index.
    ///
    /// \param  inIndex     The index of the data collection to get.
    /// \return             The data collection at the given index.
    ProjectFile::DataCollection getDataCollection(isize_t inIndex);

private:

    /// True if the project is valid, false otherwise.
    bool m_valid;

    /// The file used to store information about the project.
    std::unique_ptr<ProjectFile> m_file;
};

}
#endif // ISX_PROJECT_H
