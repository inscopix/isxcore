#ifndef ISX_EXCEPTION_H
#define ISX_EXCEPTION_H

#include <exception>
#include <vector>
#include <string>

#include "isxLog.h"

/// \def ISX_THROW(TYPE, ...)
///
/// Throws a type of exception with a message consisting of the variadic
/// arguments converted to strings.
#define ISX_THROW(TYPE, ...)\
    std::string msg = isx::internal::varArgsToString_(__VA_ARGS__);\
    ISX_LOG_ERROR(__FILE__, ":", __LINE__, ": Exception - ", msg);\
    throw TYPE(__FILE__, __LINE__, msg)

namespace isx
{
    /// The base class for exceptions.
    ///
    /// Avoid throwing this type of exception, and use one its subclasses
    /// instead.
    class Exception : public std::exception
    {
    public:

        /// Constructor
        ///
        /// \param  file    The name of the file from where the exception is thrown.
        /// \param  line    The line in the file from where the exception is thrown.
        /// \param  message The error message.
        explicit Exception(const char* file, int line, const std::string& message);

        /// Destructor
        ///
        virtual ~Exception();

        /// Retrieves the error message
        ///
        /// \return         The error message.
        virtual const char* what() const noexcept;

        /// Retrieves the file name in which failure occurs
        ///
        /// \return         The name of the file from where the exception was thrown.
        const char* getFileName() const;

        /// Return the line where failure occurs
        ///
        /// \return         The line in the file from where the exception was thrown.
        int getLine();

    protected:

        std::string m_msg;          //!< Detailed message
        std::string m_fileName;     //!< Failing file
        int m_line;                 //!< Failing line within the file
     };

    /// File I/O exception class
    ///
    /// Throw this when access to the file system fails, either to due
    /// non-existence of a file or due to lack of access permissions.
    class ExceptionFileIO : public Exception
    {
    public:

        /// Constructor
        ///
        explicit ExceptionFileIO(const char* file, int line, const std::string& message);

        /// Destructor
        ///
        virtual ~ExceptionFileIO();
    };

    /// Data I/O exception class
    ///
    class ExceptionDataIO : public Exception
    {
    public:

        /// Constructor
        ///
        /// \param  file    The name of the file from where the exception is thrown.
        /// \param  line    The line in the file from where the exception is thrown.
        /// \param  message The error message.
        explicit ExceptionDataIO(const char* file, int line, const std::string& message);

        /// Destructor
        ///
        virtual ~ExceptionDataIO();
    };

    /// User input exception class
    ///
    /// Throw this when the user provides invalid input and can adjust
    /// to provide valid input.
    class ExceptionUserInput : public Exception
    {
    public:

        /// Constructor
        ///
        /// \param  file    The name of the file from where the exception is thrown.
        /// \param  line    The line in the file from where the exception is thrown.
        /// \param  message The error message.
        explicit ExceptionUserInput(const char* file, int line, const std::string& message);

        /// Destructor
        ///
        virtual ~ExceptionUserInput();
    };

// Non API utilities.
namespace internal
{

    /// Stopping condition for recursive streamVarArgs_.
    ///
    /// \param  strm        The stream to which to append.
    void streamVarArgs_(std::ostringstream& strm);

    /// Appends variadic arguments to an output string stream.
    ///
    /// \param  strm        The stream to which to append.
    /// \param  first       The next argument to append.
    /// \param  rest        The rest of the arguments to append.
    template<typename First, typename ...Rest>
    void streamVarArgs_(std::ostringstream& strm, First && first, Rest && ...rest)
    {
        strm << std::forward<First>(first);
        streamVarArgs_(strm, std::forward<Rest>(rest)...);
    }

    /// Converts variadic arguments to a string using an output string stream.
    ///
    /// \param  rest        The arguments to convert to a string.
    /// \return             The converted string.
    template<typename ...Rest>
    std::string varArgsToString_(Rest && ...rest)
    {
        std::ostringstream strm;
        streamVarArgs_(strm, std::forward<Rest>(rest)...);
        return strm.str();
    }

} // namespace internal

} // namespace isx

#endif //ISX_EXCEPTION_H
