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

    ///// Create an empty group by its project path.
    /////
    ///// If a group with the path already exists, this will fail.
    ///// If any intermediate groups in the path do not exist, this
    ///// will create them.
    /////
    ///// \param  inPath      The path of the group to create.
    ///// \return             The group created.
    /////
    ///// \throw  isx::ExceptionDataIO    If a group with the given path
    /////                                 already exists.
    //SpGroup_t createGroup(const std::string & inPath);

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

    ///// Create a data set by its project path.
    /////
    ///// If a data set with the path already exists, this will fail.
    ///// If any intermediate groups in the path do not exist, this
    ///// will create them.
    ///// If there is an existing data set in this project with the given
    ///// file name, this will fail.
    /////
    ///// \param  inPath      The path of the data set to create.
    ///// \return             The data set created.
    /////
    ///// \throw  isx::ExceptionDataIO    If a data set with the given path
    /////                                 already exists.
    ///// \throw  isx::ExceptionFileIO    If a data set with the given file
    /////                                 name already exists.
    //SpDataSet_t createDataSet(
    //        const std::string & inPath,
    //        DataSet::Type inType,
    //        const std::string & inFileName);

    ///// Get a data set by its project path.
    /////
    ///// \param  inPath          The project path of the data set.
    ///// \return                 The retrieved data set.
    /////
    ///// \throw  isx::ExceptionDataIO    If the data set does not exist.
    //SpDataSet_t getDataSet(const std::string & inPath) const;

    /// \return     True if this project is valid.
    ///
    bool isValid() const;

    /// \return     The name of the project.
    ///
    std::string getName() const;

    /// \return The file name of this project's file.
    ///
    std::string getFileName() const;

    /// Checks if the path is a data set.
    ///
    /// \param  inPath      The path to check.
    /// \return             True if the path refers to a data set.
    bool isDataSet(const std::string & inPath) const;

    /// Checks if the path is a group.
    ///
    /// \param  inPath      The path to check.
    /// \return             True if the path refers to a group.
    bool isGroup(const std::string & inPath) const;

    /// Checks if the path is already taken by a data set or a group in this project.
    ///
    /// \param  inPath      The path to check.
    /// \return             True if there is already a data set or a group in
    ///                     this project.
    bool isPath(const std::string & inPath) const;

    /// Checks if the file is already taken by a data set in this project.
    ///
    /// \param  inFileName  The file name to check.
    /// \return             True if there is already a data set with the
    ///                     given file name in this project.
    bool isFileName(const std::string & inFileName) const;

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
