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
    /// If a file with the given file name already exists on the file system,
    /// this will fail.
    ///
    /// \param  inFileName  The file name of the project.
    /// \param  inName      The name of the project.
    ///
    /// \throw  isx::ExceptionFileIO    If the a file with the given file
    ///                                 name already exists.
    Project(const std::string & inFileName, const std::string & inName);

    /// Destructor.
    ///
    ~Project();

    /// Create a data set in this project.
    ///
    /// If a data set with the given path already exists in this project,
    /// this will fail.
    /// If there is an existing data set in this project with the given
    /// file name, this will fail.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \param  inProperties The property map for the data set to create.
    /// \return             The data set created.
    ///
    /// \throw  isx::ExceptionDataIO    If an item with the given path
    ///                                 already exists.
    /// \throw  isx::ExceptionFileIO    If a data set with the given file
    ///                                 name already exists in this project.
    DataSet * createDataSet(
            const std::string & inPath,
            DataSet::Type inType,
            const std::string & inFileName,
            const std::map<std::string, float> & inProperties = std::map<std::string, float>());

    /// Get a data set by its project path.
    ///
    /// \param  inPath      The path of the data set to retrieve.
    /// \return             The data set retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the given path.
    DataSet * getDataSet(const std::string & inPath) const;

    /// Create a group by it project path.
    ///
    /// If an item with the given path already exists in this project, this
    /// will fail.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \return             The group created.
    ///
    /// \throw  isx::ExceptionDataIO    If an with the given path already exists.
    Group * createGroup(const std::string & inPath);

    /// Get a group by its project path.
    ///
    /// \param  inPath          The project path of the group to get.
    /// \return                 The retrieved group.
    ///
    /// \throw  isx::ExceptionDataIO    If the group does not exist.
    Group * getGroup(const std::string & inPath) const;

    /// \return     The root group.
    ///
    Group * getRootGroup() const;

    /// \return     The group of original data.
    ///
    Group * getOriginalGroup() const;

    /// \return     The group of processed data.
    ///
    Group * getProcessedGroup() const;

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
    std::unique_ptr<Group> m_root;

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

    /// Checks if the file name is already used by a data set in this group's tree.
    ///
    /// \param  inFileName  The file name to check.
    /// \return             True if this group's tree contains a data set with
    ///                     given file name.
    bool isFileName(const std::string & inFileName);

};

}
#endif // ISX_PROJECT_H
