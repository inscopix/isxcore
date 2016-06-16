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
    std::string msg = isx::internal::varArgsToString(__VA_ARGS__);\
    std::string file = isx::internal::baseName(__FILE__);\
    ISX_LOG_ERROR(file, ":", __LINE__, ": Exception - ", msg);\
    throw TYPE(file, __LINE__, msg)

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
        explicit Exception(const std::string& file, int line, const std::string& message);

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
        const std::string& getFileName() const;

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
        explicit ExceptionFileIO(const std::string& file, int line, const std::string& message);

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
        explicit ExceptionDataIO(const std::string& file, int line, const std::string& message);

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
        explicit ExceptionUserInput(const std::string& file, int line, const std::string& message);

        /// Destructor
        ///
        virtual ~ExceptionUserInput();
    };

// Non API utilities.
namespace internal
{

    /// Stopping condition for recursive streamVarArgs.
    ///
    /// \param  strm        The stream to which to append.
    void streamVarArgs(std::ostringstream& strm);

    /// Appends variadic arguments to an output string stream.
    ///
    /// \param  strm        The stream to which to append.
    /// \param  first       The next argument to append.
    /// \param  rest        The rest of the arguments to append.
    template<typename First, typename ...Rest>
    void streamVarArgs(std::ostringstream& strm, First && first, Rest && ...rest)
    {
        strm << std::forward<First>(first);
        streamVarArgs(strm, std::forward<Rest>(rest)...);
    }

    /// Converts variadic arguments to a string using an output string stream.
    ///
    /// \param  rest        The arguments to convert to a string.
    /// \return             The converted string.
    template<typename ...Rest>
    std::string varArgsToString(Rest && ...rest)
    {
        std::ostringstream strm;
        streamVarArgs(strm, std::forward<Rest>(rest)...);
        return strm.str();
    }

} // namespace internal

} // namespace isx

#endif //ISX_EXCEPTION_H
