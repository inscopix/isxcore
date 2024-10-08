#ifndef ISX_PATH_UTILS_H
#define ISX_PATH_UTILS_H

#include "isxCore.h"

#include <string>
#include <vector>

namespace isx
{

/// Get the base name of a path.
///
/// The base name does not include the extension nor the directory name.
///
/// For example:
/// - "movie.isxd" -> "movie"
/// - "outputs" -> "outputs"
/// - "outputs/movie.isxd" -> "movie"
/// - "outputs/day1" -> "day1"
///
/// \param  inPath  The path from which to get the base name.
/// \return         The base name of the path.
std::string getBaseName(const std::string & inPath);

/// Get the file name of a path.
///
/// The file name does not include the directory name, but does include the
/// extension.
/// This name is perhaps misleading because this will work for
/// directories, but in that case should act as getBaseName.
///
/// For example:
/// - "movie.isxd" -> "movie.isxd"
/// - "outputs" -> "outputs"
/// - "outputs/movie.isxd" -> "movie.isxd"
/// - "outputs/day1" -> "day1"
///
/// \param  inPath  The path from which to get the file name.
/// \return         The file name of the path.
std::string getFileName(const std::string & inPath);

/// Get the directory name of a path.
///
/// For example:
/// - "movie.isxd" -> "." (to signify the current directory)
/// - "outputs" -> "." (to signify the current directory)
/// - "outputs/movie.isxd" -> "outputs"
/// - "outputs/day1" -> "outputs"
///
/// \param  inPath  The path from which to get the base name.
/// \return         The directory name of the path.
std::string getDirName(const std::string & inPath);

/// Get the extension of a path.
///
/// For example:
/// - "movie.isxd" -> "isxd"
/// - "movie.isxd.gz" -> "gz"
/// - "outputs" -> ""
/// - "outputs/movie.isxd" -> "isxd"
/// - "outputs/day1" -> ""
///
/// \param  inPath  The path from which to get the extension.
/// \return         The extension of the path.
std::string getExtension(const std::string & inPath);

/// Get the tokens of a path delimited by '/'.
///
/// If the first character is a '/' this will include that as the
/// first token.
///
/// \param  inPath  The path to get tokens from.
/// \return         The tokens of a path delimited by '/'.
std::vector<std::string> getPathTokens(const std::string & inPath);

/// \return whether a path is relative or not
/// \param inPath the path of interest
bool isRelative(const std::string &inPath);

/// Get a path relative to a directory name.
///
/// If the path is on a Windows filesystem and is on a different drive
/// than the directory, this does ... ?
///
/// \param  inPath      The path to make relative.
/// \param  inDirName   The directory from which to get the relative path.
/// \return             The relative path from the given directory.
std::string getRelativePath(
        const std::string & inPath,
        const std::string & inDirName);

/// Get the absolute path of a file/directory on the file system.
///
/// If the path cannot be found on the file system, this will return
/// a copy of the given path.
///
/// \param  inPath      The path.
/// \return             The absolute version of the given path.
std::string getAbsolutePath(const std::string & inPath);

/// Get the canonical path of a file/directory on the file system.
///
/// If the path cannot be found on the file system, this will return
/// an empty string.
///
/// \param  inPath      The path.
/// \return             The canonical version of the given path.
std::string getCanonicalPath(const std::string & inPath);

/// Check if a path exists on the file system.
///
/// \param  inPath      The path to check for existence.
/// \return             True if the path exists on the file system.
bool pathExists(const std::string & inPath);

/// Returns all fileNames from specified directory.
///
/// \param  inPath      The path to check for existence.
/// \return             Array of file path.
std::vector<std::string> getAllDirFiles(const std::string & inPath);

/// Append a separator and a zero padded non-negative number to a path.
///
/// \param  inPath      The path to which to append.
/// \param  inNumber    The number to append.
/// \param  inWidth     The width of the number string to append.
/// \return             The resulting path.
std::string appendNumberToPath(
        const std::string & inPath,
        const isize_t inNumber,
        const isize_t inWidth);

/// Makes a directory on the file system with the given path.
///
/// \param  inPath  The path of the directory to make.
/// \return         True if making the directory was successful.
bool makeDirectory(const std::string & inPath);

/// Remove a directory recursively.
///
/// \param  inPath  The path of the directory to remove.
/// \return         True if removing the directory was completely successful.
bool removeDirectory(const std::string & inPath);

/// Copy a single file without overwriting the destination.
///
/// \return True if the copy was successful, false otherwise.
///         The copy will not be successful if the destination already exists.
bool copyFile(const std::string & inSourcePath, const std::string & inDestPath);

/// Make a unique path on the file system by appending a zero padded
/// non-negative number.
///
/// \param  inPath  The requested file path.
/// \param  inWidth The width of the number to append.
/// \return         The unique file path.
std::string makeUniqueFilePath(const std::string & inPath, const isize_t inWidth = 3);

/// What is the available disk space in bytes for the given path
///
/// \param  dirPath      The requested file path.
/// \param  outRootDir   root dir
/// \return         The number of available bytes.
long long availableNumberOfBytesOnVolume(const std::string & dirPath, std::string & outRootDir);

/// Removes given file paths. Failure to remove a file is silent and will not error.
///
/// This is used by some algorithms and exporters to clean up multiple
/// output files on cancellation or error.
///
/// \param  inFilePaths     The paths of the files to remove.
void removeFiles(const std::vector<std::string> & inFilePaths);

} // namespace isx

#endif // ISX_PATH_UTILS_H
