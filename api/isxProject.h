#ifndef ISX_PROJECT_H
#define ISX_PROJECT_H

#include "isxDataSet.h"
#include "isxGroup.h"
#include "isxSeries.h"

#include <string>
#include <memory>

namespace isx
{

/// Encapsulates a mosaic project and all items associated with it.
///
/// An item can either be a group, series or data set.
///
/// The project has one root group that can contain heterogeneous project items.
/// Currently, these are either series or data sets.
///
/// Each item has a name and a type, and is uniquely identified in a project
/// by its absolute path from the root group, where the '/' character as a
/// delimiter.
///
/// Any data set is also uniquely identified by its file name on the actual
/// filesystem - i.e. two or more datasets in a project cannot refer to the
/// same file.
///
/// \sa ProjectItem, Group, Series, DataSet
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
    ///
    /// \throw  ExceptionFileIO If the project file cannot be read.
    /// \throw  ExceptionDataIO If the project file cannot be parsed.
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
    /// \throw  ExceptionFileIO If a file with the given file name already
    ///                         exists or it cannot be written to.
    /// \throw  ExceptionDataIO If the project data cannot be serialized.
    Project(const std::string & inFileName, const std::string & inName);

    /// Write the file to disk.
    ///
    void save();

    /// Get an item by its project path.
    ///
    /// \param  inPath  The project path of the item to get.
    /// \return         The retrieved item.
    ///
    /// \throw  ExceptionDataIO If the item does not exist.
    ProjectItem * getItem(const std::string & inPath) const;

    /// Remove an item by its project path.
    ///
    /// \param  inPath  The path of the item to remove.
    ///
    /// \throw  ExceptionDataIO If the item does not exist.
    void removeItem(const std::string & inPath) const;

    /// Move an item into a new destination/parent item.
    ///
    /// \param  inSrc       The path of the item to move.
    /// \param  inDest      The path of the destination item.
    /// \param  inIndex     The index in the destination at which to insert, or -1
    ///                     to signify the end.
    ///
    /// \throw  ExceptionDataIO     If one of the items does not exist.
    /// \throw  ExceptionFileIO     If a data set file needs to be and cannot be read.
    /// \throw  ExceptionSeries     If the item cannot be moved due to series constraints.
    void moveItem(
            const std::string & inSrc,
            const std::string & inDest,
            const int inIndex = -1);

    /// Create a data set in this project.
    ///
    /// If the data set is being created in a series, then this will
    /// check that the data set being created is consistent with data sets
    /// already in that group. If it is not, this will error.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \param  inProperties The property map for the data set to create.
    /// \return             The data set created.
    ///
    /// \throw  ExceptionDataIO If an item with the given path already exists.
    /// \throw  ExceptionFileIO If a data set with the given file name already
    ///                         exists in this project.
    /// \throw  ExceptionSeries If this data set is not consistent with existing
    ///                         data sets in a series.
    DataSet * createDataSet(
            const std::string & inPath,
            const DataSet::Type inType,
            const std::string & inFileName,
            const DataSet::Properties & inProperties = DataSet::Properties());

    /// Create a series by its project path.
    ///
    /// \param  inPath      The path of the series to create.
    /// \param  inIndex     The index at which to insert this in the root,
    ///                     or -1 if it should be inserted at the end.
    /// \return             The series created.
    ///
    /// \throw  ExceptionDataIO    If an item with the path already exists.
    Series * createSeries(const std::string & inPath, const int inIndex = -1);

    /// Flatten a series by its project path.
    ///
    /// This will remove the series and move its data sets into its parent.
    ///
    /// \param  inPath  The path of the series to flatten.
    ///
    /// \throw  ExceptionDataIO If a series does not exist.
    void flattenSeries(const std::string & inPath);

    /// \return The root group.
    ///
    Group * getRootGroup() const;

    /// \return True if this project is valid.
    ///
    bool isValid() const;

    /// \return The name of this project.
    ///
    std::string getName() const;

    /// \return The file name of this project's file.
    ///
    std::string getFileName() const;

    /// Sets the file name of this project's file.
    ///
    void setFileName(const std::string & inFileName);

    /// Create a unique path in this project given a requested one.
    ///
    /// Uniqueness is achieved by appending an underscore and a zero-padded
    /// number of width 3 to the requested path.
    ///
    /// For Example, if a project already contains an item with the path
    /// "/movie-pp", then the result of
    ///     createUniquePath("/movie-pp")
    /// will be "movie-pp_000".
    ///
    /// \param  inPath  The requested path in this project.
    /// \return         A path that is unique in this project based on the given one.
    std::string createUniquePath(const std::string & inPath) const;

    /// \return whether the file has unsaved changes.
    ///
    bool isModified() const;

private:

    /// True if the project is valid, false otherwise.
    bool m_valid;

    /// The name of the project.
    std::string m_name;

    /// The root group of the project.
    std::shared_ptr<Group> m_root;

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

    /// Checks if the file name is already used by a data set in this project.
    ///
    /// \param  inFileName  The file name to check.
    /// \return             True if this project contains a data set with given file name.
    bool isFileName(const std::string & inFileName);

    /// Checks if the path is already used in this project.
    ///
    /// \param  inPath      The path to check.
    /// \return             True if this project contains a data set with
    ///                     given path.
    bool isPath(const std::string & inPath) const;

    /// Sets all the groups/datasets in the project as unmodified
    ///
    void setUnmodified();

    /// \return All items in the project.
    ///
    std::vector<ProjectItem *> getAllItems() const;

    /// \param  inItem  The item of which to get all child items recursively.
    /// \return         The recursively retrieved child items.
    std::vector<ProjectItem *> getAllItems(const ProjectItem * inItem) const;
};

}
#endif // ISX_PROJECT_H
