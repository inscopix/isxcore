#ifndef ISX_PATH_UTILS_H
#define ISX_PATH_UTILS_H

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
/// - "outputs/movie.isxd" -> "outputs/movie"
/// - "outputs/day1" -> "outputs/day1"
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
/// - "outputs/day1" -> "outputs/day1"
///
/// \param  inPath  The path from which to get the file name.
/// \return         The file name of the path.
std::string getFileName(const std::string & inPath);

/// Get the directory name of a path.
///
/// For example:
/// - "movie.isxd" -> ""
/// - "outputs" -> ""
/// - "outputs/movie.isxd" -> "outputs"
/// - "outputs/day1" -> "outputs"
///
/// \param  inPath  The path from which to get the base name.
/// \return         The directory name of the path.
std::string getDirName(const std::string & inPath);

/// Get the extension of a path.
///
/// For example:
/// - "movie.isxd" -> ""
/// - "movie.isxd.gz" -> "isxd.gz"
/// - "outputs" -> ""
/// - "outputs/movie.isxd" -> "outputs"
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

/// Get the writable directory name for Inscopix data.
///
/// The directory will be created if it doesn't exist.
///
/// \return     A writable directory name for Inscopix data.
std::string getWritableDirName();

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

/// Check if a patch exists on the file system.
///
/// \param  inPath      The path to check for existence.
/// \return             True if the path exists on the file system.
bool pathExists(const std::string & inPath);

} // namespace isx

#endif // ISX_PATH_UTILS_H
