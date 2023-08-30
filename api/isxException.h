#ifndef ISX_EXCEPTION_H
#define ISX_EXCEPTION_H

#include <exception>
#include <vector>
#include <string>

#include "isxLog.h"

#ifdef ISX_DONT_LOG_EXCEPTIONS
#define ISX_LOG_EXCEPTION(...)
#else
#define ISX_LOG_EXCEPTION ISX_LOG_ERROR
#endif

/// \def ISX_THROW(TYPE, ...)
///
/// Throws a type of exception with a message consisting of the variadic
/// arguments converted to strings.
#define ISX_THROW(TYPE, ...)\
    std::string msg = isx::internal::varArgsToString(__VA_ARGS__);\
    std::string file = isx::internal::baseName(__FILE__);\
    ISX_LOG_EXCEPTION(file, ":", __LINE__, ": Exception - ", msg);\
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
        ~Exception() override;

        /// Retrieves the error message
        ///
        /// \return         The error message.
        const char* what() const noexcept override;

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
        ~ExceptionFileIO() override;
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
        ~ExceptionDataIO() override;
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
        ~ExceptionUserInput() override;
    };

    /// Exception related to adding a data set to a series.
    ///
    class ExceptionSeries : public Exception
    {
    public:

        /// Constructor
        ///
        /// \param  file    The name of the file from where the exception is thrown.
        /// \param  line    The line in the file from where the exception is thrown.
        /// \param  message The error message.
        explicit ExceptionSeries(const std::string& file, int line, const std::string& message);

        /// Destructor
        ///
        ~ExceptionSeries() override;
    };

    /// Returns a string error message describing the system error code errnum
    ///
    std::string getSystemErrorString();

} // namespace isx

#endif //ISX_EXCEPTION_H
