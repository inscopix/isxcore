#ifndef ISX_PROJECT_H
#define ISX_PROJECT_H

#include "isxDataSet.h"
#include "isxGroup.h"

#include <string>
#include <memory>

namespace isx
{

/// Encapsulates a mosaic project and owns all data objects associated with it.
///
/// The project is similar to a filesystem that stores groups, which are like
/// directories, that contain sub-groups and datasets, which are like files.
///
/// In the current state of the project (i.e. ignoring history) any group
/// or dataset is also uniquely identified by its absolute path from the root
/// of the project where the / character is used as a group delimiter.
/// Any dataset is also uniquely identified by its file name on the actual
/// filesystem - i.e. two or more datasets in a project cannot refer to the
/// same file.
class Project
{
public:

    /// Empty constructor.
    ///
    /// Creates a valid c++ object but invalid project.
    Project();

    /// Construct a project from an existing project file.
    ///
    /// \param  inFileName  The file name of the project.
    Project(const std::string & inFileName);

    /// Construct a new project with the given file name and name.
    ///
    /// This will be written to disk either when calling write() or on
    /// destruction.
    ///
    /// \param  inFileName  The file name of the project.
    /// \param  inName      The name of the project.
    Project(const std::string & inFileName, const std::string & inName);

    /// Destructor.
    ///
    ~Project();

    /// Get a data set by its project path.
    ///
    /// \param  inPath          The project path of the data set.
    /// \return                 The retrieved data set.
    ///
    /// \throw  isx::ExceptionDataIO    If the data set does not exist.
    SpGroup_t getGroup(const std::string & inPath) const;

    /// \return     The root group.
    ///
    SpGroup_t getRootGroup() const;

    /// \return     The group of original data.
    ///
    SpGroup_t getOriginalGroup() const;

    /// \return     The group of output data.
    ///
    SpGroup_t getOutputGroup() const;

    /// \return     True if this project is valid.
    ///
    bool isValid() const;

    /// \return     The name of the project.
    ///
    std::string getName() const;

    /// \return The file name of this project's file.
    ///
    std::string getFileName() const;

private:

    /// True if the project is valid, false otherwise.
    bool m_valid;

    /// The name of the project.
    std::string m_name;

    /// The root group of the project.
    SpGroup_t m_root;

    /// The file name of the project file.
    std::string m_fileName;

    /// Read this project from its file.
    ///
    /// This requires the file name to already be set.
    void read();

    /// Write this project to its file.
    ///
    /// This requires the file name to already be set.
    void write() const;

};

/// The type of a shared pointer to a project.
typedef std::shared_ptr<Project> SpProject_t;

}
#endif // ISX_PROJECT_H
