#ifndef ISX_CORE_H
#define ISX_CORE_H

#include <stddef.h>
#include <vector>

#include <ostream>
#include <fstream>
#include <sstream>

namespace isx
{
    /// The type of all sizes, lengths and indices
    typedef size_t isize_t;

    /// \cond doxygen chokes on enum class inside of namespace
    /// The data types of values.
    enum class DataType
    {
        U16 = 0,
        F32,
        U8,
        RGB888,
    };
    /// \endcond doxygen chokes on enum class inside of namespace

    /// Get the size of a data type in bytes.
    ///
    /// \param  inDataType  The data type.
    /// \return             The size of the given data type in bytes.
    isize_t getDataTypeSizeInBytes(DataType inDataType);

    /// \return a string that describes the data type 
    /// \param inDataType the detaype enum 
    std::string getDataTypeString(DataType inDataType);

    /// Overload of ostream << operator for DataType.
    ///
    /// \param   inStream   The output stream to which to print.
    /// \param   inDataType The data type to output.
    /// \return             The modified output stream.
    std::ostream & operator<<(
            std::ostream & inStream,
            DataType inDataType);

    /// Convert a non-negative number to a string.
    ///
    /// \param  inNumber    The number to convert.
    /// \param  inWidth     The minimum width of the string.
    /// \return             The resulting string.
    std::string convertNumberToPaddedString(
            const size_t inNumber,
            const size_t inWidth);

    void CoreInitialize(const std::string & inLogFileName = std::string());
    bool CoreIsInitialized();
    void CoreShutdown();

    int CoreVersionMajor();
    int CoreVersionMinor();
    int CoreVersionPatch();
    int CoreVersionBuild();
    bool isBeta();
    std::string CoreVersionString();

    /// \return     The version numbers in a vector.
    ///
    std::vector<int> CoreVersionVector();

    /// \return     The host name of the machine this is running on.
    ///
    std::string getHostName();

    /// \param  inString    The string to split.
    /// \param  inDelim     The delimiter to split with.
    /// \return             The components of string split by the given delimiter.
    std::vector<std::string> splitString(const std::string & inString, const char inDelim);

    /// Trim a string by removing leading and trailing spaces.
    ///
    /// \param  inString    The string to trim.
    /// \return             The trimmed string.
    std::string trimString(const std::string & inString);

    /// Read a line from a file in a cross-platform way w.r.t. line endings.
    ///
    /// \param  inStream    The input file stream to read a line from.
    /// \param  outLine     The line without characters indicating its end ('\\n' and '\\r').
    /// \return             The modified input stream, which can be used as a bool-like
    ///                     value to check if the stream is still good.
    std::ifstream & getLine(std::ifstream & inStream, std::string & outLine);

    /// Use this function to copy C++ strings to C strings instead of
    /// the complicated and confusion str(n)cpy(_s).
    ///
    /// This will truncate the string if it's too long to fit in the
    /// C string destination (noting that the C string will always
    /// be null terminated).
    ///
    /// \param  inSource    The C++ string to copy.
    /// \param  inDest      The C string to which to write.
    /// \param  inDestCapacity  The size of the C string destination.
    void copyCppStringToCString(const std::string & inSource, char * inDest, size_t inDestCapacity);

    void closeFileStreamWithChecks(std::fstream & inFile, const std::string & inFileName);

    /// Define a file path based on a reference file path and a given suffix.
    ///
    /// For example, makeOutputFilePath("/a/b/c.isxd", "-crop.csv") will
    /// return "/a/b/c-crop.csv".
    ///
    /// This does not attempt to find a unique path on the file system.
    ///
    /// \param  inFilePath  The reference file path.
    /// \param  inSuffix    The suffix to append to the file after the base name.
    std::string makeOutputFilePath(const std::string & inFilePath, const std::string & inSuffix);

    /// \param  inStr   The string to convert.
    /// \return         The string with lowercase characters only.
    std::string toLower(const std::string & inStr);

} // namespace isx

#endif // def ISX_CORE_H
