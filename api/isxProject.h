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
/// of the project where the '/' character is used as a group delimiter.
/// Additionally, each data set that is imported into the project (using the
/// importDataSet method) is added to the "OriginalData" group.
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

    /// Write the file to disk.
    ///
    void save();

    /// Import a data set into this project.
    ///
    /// This will add a dataset to the original data group and will also
    /// create a data set group in the project data group.
    /// If a data set with the given name already exists in this project,
    /// this will fail.
    /// If there is an existing data set in this project with the given
    /// file name, this will fail.
    ///
    /// \param  inName      The name of the data set to create.
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \param  inProperties The property map for the data set to create.
    /// \return             The data set created.
    ///
    /// \throw  isx::ExceptionDataIO    If an item with the given name
    ///                                 already exists.
    /// \throw  isx::ExceptionFileIO    If a data set with the given file
    ///                                 name has already been imported into
    ///                                 this project.
    DataSet * importDataSet(
            const std::string & inName,
            DataSet::Type inType,
            const std::string & inFileName,
            const DataSet::Properties & inProperties = DataSet::Properties());

    /// Create a data set in this project.
    ///
    /// This will create a data set group in the root group, will
    /// create a data set in that group and will create a derived data set
    /// group associated with that data set in the same group.
    /// If a data set with the given path already exists in this project,
    /// this will fail.
    /// If there is an existing data set in this project with the given
    /// file name, this will fail.
    /// If the data set is being created in a series group, then this will
    /// check that the data set being created is consistent with data sets
    /// already in that group. If it is not, this will error.
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
    /// \throw  isx::ExceptionSeries    If this data set is not consistent with
    ///                                 existing data sets in a series.
    DataSet * createDataSet(
            const std::string & inPath,
            DataSet::Type inType,
            const std::string & inFileName,
            const DataSet::Properties & inProperties = DataSet::Properties());

    /// Get a data set by its project path.
    ///
    /// \param  inPath      The path of the data set to retrieve.
    /// \return             The data set retrieved.
    ///
    /// \throw  isx::ExceptionDataIO    If there is no data set with the given path.
    DataSet * getDataSet(const std::string & inPath) const;

    /// Create a group by its project path.
    ///
    /// If an item with the given path already exists in this project, this
    /// will fail.
    ///
    /// \param  inPath      The path of the data set to create.
    /// \param  inType      The type of group to create.
    /// \param  inIndex     The index at which to insert this in its parent,
    ///                     or -1 if it should be inserted at the end.
    /// \return             The group created.
    ///
    /// \throw  isx::ExceptionDataIO    If a group with the given path already exists.
    /// \throw  ExceptionSeries         If a non-dataset group is being created in a series group.
    Group * createGroup(
            const std::string & inPath,
            const isx::Group::Type inType = isx::Group::Type::GENERAL,
            const int inIndex = -1);

    /// Move a group by its project path into a new parent.
    ///
    /// \param  inSrcs   The paths of the source groups to move.
    /// \param  inDest   The path of the destination/parent group.
    /// \param  inIndex  The index in the destination in which to insert
    ///                  the sub-group.
    /// \throw  isx::ExceptionDataIO    If there is no group with either path.
    /// \throw  isx::ExceptionSeries    If the group cannot be added due to series constraints.
    void moveGroups(
            const std::vector<std::string> & inSrcs,
            const std::string & inDest,
            const int inIndex = -1);

    /// Flatten a group by its project path.
    ///
    /// This will remove the group and moves its members into its parent.
    ///
    /// \param  inPath      The path of the data set to flatten.
    /// \throw  isx::ExceptionDataIO    If there is no group with the given path
    ///                                 or it refers to the root group.
    void flattenGroup(const std::string & inPath);

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

    /// \return     The group of original data that have been imported into this project.
    ///
    Group * getOriginalDataGroup() const;

    /// \return     True if this project is valid.
    ///
    bool isValid() const;

    /// \return     The name of the project.
    ///
    std::string getName() const;

    /// \return The file name of this project's file.
    ///
    std::string getFileName() const;

    /// Sets the file name of this project's file.
    ///
    void setFileName(const std::string & inFileName);

    /// \param  inPath  The requested path in this project.
    /// \return         A path that is unique in this project from the given one.
    ///                 Uniqueness is achieved by appending zero-padded numbers to
    ///                 the given path.
    std::string createUniquePath(const std::string & inPath) const;

    /// \return whether the file has unsaved changes.
    ///
    bool isModified() const;

    /// \return True if the given path represents a Group.
    ///
    bool isGroup(const std::string & inPath) const;

    /// Checks that data sets are consistent for a series.
    ///
    /// This uses the first data set as reference and checks its consistency
    /// with the other data sets.
    ///
    /// \param  inDataSets  The data sets to check.
    /// \throw  isx::ExceptionSeries    If one of the data sets is not consistent
    ///                                 with the first one.
    static void checkDataSetsForSeries(const std::vector<const DataSet *> & inDataSets);

private:

    /// True if the project is valid, false otherwise.
    bool m_valid;

    /// The name of the project.
    std::string m_name;

    /// The root group of the project.
    std::unique_ptr<Group> m_root;

    /// The group of original data that have been imported into this.
    std::unique_ptr<Group> m_originalData;

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
    /// \return             True if this project contains a data set with
    ///                     given file name.
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

    /// Checks that a data set can be added to a series.
    ///
    /// This simply checks that the type of the data set is supported.
    ///
    /// \param  inType      The type of data set that's being added.
    /// \throw  isx::ExceptionSeries    If the data set can't be added to a series.
    static void checkDataSetForSeries(const DataSet::Type inType);

    /// Checks that a data set is consistent with an existing reference data set in a series.
    ///
    /// This reads the data set meta data from both files and checks consistency.
    ///
    /// \param  inRef       The reference data set from a series.
    /// \param  inType      The type of data set that's being added.
    /// \param  inFileName  The file name of the data set that's being added.
    /// \throw  isx::ExceptionSeries    If the data set is not consistent with an
    ///                                 the reference data set.
    static void checkDataSetForSeries(
            const DataSet * inRef,
            const DataSet::Type inType,
            const std::string & inFileName);

    /// Checks that a data set group can be added to a series group.
    ///
    /// This checks that the new data set is consistent with each of the existing
    /// members.
    ///
    /// \param  inSeries    The series to which to add.
    /// \param  inType      The type of data set that's being added.
    /// \param  inFileName  The file name of the data set.
    /// \throw  isx::ExceptionSeries    If the data set is not consistent
    ///                                 with any of the existing ones.
    static void checkDataSetForSeries(
           const Group * inSeries,
           const DataSet::Type inType,
           const std::string & inFileName);

    /// Find the insertion index for a data set in a series ordered by time.
    ///
    /// This assumes that the data set has already been checked that it can
    /// be added to a series.
    ///
    /// \param  inSeries    The series group.
    /// \param  inType      The data set to be added.
    /// \param  inFileName  The file name of the data set.
    /// \return             The index where the new data set should be added
    ///                     so that the series is ordered, or -1 if it cannot
    ///                     be added.
    static int findSeriesIndex(
            const Group * inSeries,
            const DataSet::Type inType,
            const std::string & inFileName);

};

}
#endif // ISX_PROJECT_H
