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

    /// Destructor
    ///
    ~Project();

    /// Write the file to disk.
    ///
    void save();

    /// Write filename.isxp.tmp to file
    void saveTmp();

    /// Create a data set at the root of this project.
    ///
    /// \param  inName      The name of the DataSet to create
    /// \param  inType      The type of the data set to create.
    /// \param  inFileName  The file name of the data set to create.
    /// \param  inHistory   The historical details for the dataset
    /// \param  inProperties The property map for the data set to create.
    /// \return The new Series object that was created and inserted.
    ///
    /// \throw  ExceptionDataIO If an item with the given path already exists.
    /// \throw  ExceptionFileIO If a data set with the given file name already
    ///                         exists in this project.
    SpSeries_t
    importDataSetInRoot(
        const std::string & inName,
        const DataSet::Type inType,
        const std::string & inFileName,
        const HistoricalDetails & inHistory,
        const DataSet::Properties & inProperties = DataSet::Properties());

    /// Create a dataset in a series.
    ///
    SpSeries_t
    importDataSetInSeries(
        const std::string & inParentId,
        const std::string & inName,
        const DataSet::Type inType,
        const std::string & inFileName,
        const HistoricalDetails & inHistory,
        std::string & outErrorMessage,
        const DataSet::Properties & inProperties = DataSet::Properties());

    /// Create a series in root of this project
    ///
    /// \param  inName      The name of the series to create.
    /// \return The new Series object that was created and inserted.
    ///
    /// \throw  ExceptionDataIO    If an item with the path already exists.
    SpSeries_t
    createSeriesInRoot(const std::string & inName);

    /// Check if a series can be flattened.
    ///
    /// \param  inSeries    The series to flatten.
    /// \param  outMessage  The error message the series cannot be flattened.
    /// \return             True if the path represents a series that can be flattened.
    bool canFlattenSeries(
            Series * inSeries,
            std::string & outMessage) const;

    /// Flatten a series.
    ///
    /// This will remove the series and move its data sets into its parent.
    ///
    /// \param  inSeries      The series to flatten.
    ///
    /// \throw  ExceptionDataIO If a series does not exist.
    void flattenSeries(Series * inSeries);

    /// Look up a Series by its identifier
    /// \return Series matching the given identifier
    /// \param inId The Series identifier
    Series *
    findSeriesFromIdentifier(const std::string & inId) const;

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

    /// \return the path for data files
    ///
    std::string getDataPath() const;

    /// \return the path for the project file
    ///
    std::string getProjectPath() const;

    /// Sets the file name of this project's file.
    ///
    void setFileName(const std::string & inFileName, bool inFromTemporary);

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

    /// Discard the project and delete all data files
    ///
    void discard();

    /// \return     True if all data files referenced in this project exist on
    ///             the filesystem, false otherwise.
    bool allDataFilesExist() const;

    /// \return     The series in this project with some missing data files.
    ///
    std::vector<Series *> getSeriesWithMissingFiles() const;

    /// Locate missing series' files in the given directory.
    /// This will update the series' file paths in place.
    ///
    /// \param  inDirectory The directory in which to locate the files.
    /// \param  inSeries    The series to locate.
    /// \param  outLocated  The series that were found.
    /// \return             True if all series' files were found.
    bool locateMissingFiles(
            const std::string & inDirectory,
            const std::vector<Series *> & inSeries,
            std::vector<Series *> & outLocated);

private:

    /// True if the project is valid, false otherwise.
    bool m_valid;

    /// The name of the project.
    std::string m_name;

    /// The root group of the project.
    std::shared_ptr<Group> m_root;

    /// The file name of the project file.
    std::string m_fileName;

    /// The file version
    const static size_t s_version = 0;

    /// Read this project from its file.
    ///
    /// This requires the file name to already be set.
    void read();

    /// Write this project to its file.
    /// \param inFilename a filename other than m_fileName to be used. .
    void write(const std::string & inFilename) const;

    /// \return the temporary filename
    ///
    std::string getTmpFileName() const;

    /// Throw if the file name is already used in this project.
    ///
    /// \param  inFileName          The file name to check.
    /// \throw  ExceptionFileIO     If the project contains a data set with
    ///                             the given file name.
    void throwIfIsFileName(const std::string & inFileName);

    /// Sets all the groups/datasets in the project as unmodified
    ///
    void setUnmodified();

    /// Initialize data directory
    ///
    void initDataDir();

    /// \return All Series in the project not including those that are members of
    ///         a series (i.e. children only).
    std::vector<Series *>
    getAllSeries() const;

    /// \param  inItem  The Group of which to get all contained Series recursively.
    /// \return         The recursively retrieved member Series.
    std::vector<Series *>
    getAllSeries(const Group * inItem) const;

    /// \param  inItem  The Series of which to get all derived Series recursively.
    /// \return         The recursively retrieved children of Series.
    std::vector<Series *>
    getAllSeries(Series * inItem) const;
};

}
#endif // ISX_PROJECT_H
