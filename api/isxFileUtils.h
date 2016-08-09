#ifndef ISX_FILE_UTILS_H
#define ISX_FILE_UTILS_H

#include <string>
#include <vector>

namespace isx
{

/// Get the base name of a path.
///
/// The base name does not include the extension nor the path.
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

/// \return     True if path exists.
///
bool doesPathExist(const std::string & inPath);

/// Get the tokens of a path delimited by '/'.
///
/// If the first character is a '/' this will include that as the
/// first token.
///
/// \param  inPath  The path to get tokens from.
/// \return         The tokens of a path delimited by '/'.
std::vector<std::string> getPathTokens(const std::string & inPath);

/// Create a path from its tokens by inserting '/' in between each token.
///
/// \param  inPathTokens    The path tokens.
/// \return                 The path name created from the given tokens.
std::string createPath(const std::vector<std::string> & inPathTokens);

/// Returns a writable directory name for Inscopix data.
///
/// The directory will be created if it doesn't exist.
///
/// \return     A writable directory name for Inscopix data.
std::string getWritableDirName();

} // namespace isx

#endif // ISX_FILE_UTILS_H
