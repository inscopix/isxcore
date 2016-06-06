#ifndef ISX_EXCEPTION_H
#define ISX_EXCEPTION_H

#include <exception>
#include <vector>
#include <string>

#include "isxLog.h"


namespace isx
{
    /// The base class for exceptions
    ///
    class Exception : public std::exception
    {
    public:
        
        /// An enumerator describing the types of exceptions that exist
        //
        enum ExceptionTypes
        {
            ISX_EXCEPTION_FILEIO = 0,       
            ISX_EXCEPTION_DATAIO
        };

        /// A vector containing the string names of the exception types
        ///
        static const std::vector<std::string> ExceptionTypeNames;

        /// Default message for exceptions
        static const std::string DEFAULT_MSG;

        /// Constructor
        ///
        explicit Exception(const std::string& function, const std::string& message) :
            m_msg(message),
            m_funcName(function)
        {}

        /// Destructor
        ///
        virtual ~Exception() throw () {}

        /// Retrieves the error message
        virtual const char* what() const throw () 
        {
            return m_msg.c_str();
        }

        /// Retrieves the function name in which failure occurs
        ///
        const char* getFuncName() const
        {
            return m_funcName.c_str();
        }

        /// Retrieves the exception type
        ///
        const char* getType() const
        {
            return m_exceptionType.c_str();
        }

    protected:
        
        std::string m_funcName;
        std::string m_msg;
        std::string m_exceptionType;
    };

    /// File I/O exception class
    ///
    class ExceptionFileIO : public Exception
    {
    public:
        
        /// Constructor
        ///
        explicit ExceptionFileIO(const std::string& function, const std::string& message = DEFAULT_MSG) :
            Exception(function, message)
        {
            m_exceptionType = ExceptionTypeNames[ISX_EXCEPTION_FILEIO];
            ISX_LOG_ERROR(m_exceptionType + " in " + m_funcName + " - " + m_msg);
        }
        
        /// Destructor
        ///
        ~ExceptionFileIO() {}
    };

    /// Data I/O exception class
    ///
    class ExceptionDataIO : public Exception
    {
    public:

        /// Constructor
        ///
        explicit ExceptionDataIO(const std::string& function, const std::string& message = DEFAULT_MSG) :
            Exception(function, message)
        {
            m_exceptionType = ExceptionTypeNames[ISX_EXCEPTION_DATAIO];
            ISX_LOG_ERROR(m_exceptionType + " in " + m_funcName + " - " + m_msg);
        }
        
        /// Destructor
        ///
        ~ExceptionDataIO() {}
    };
}


#endif //ISX_EXCEPTION_H
