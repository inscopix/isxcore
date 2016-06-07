#ifndef ISX_EXCEPTION_H
#define ISX_EXCEPTION_H

#include <exception>
#include <vector>
#include <string>

#include "isxLog.h"

#define ISX_THROW_EXCEPTION_FILEIO(MSG) throw isx::ExceptionFileIO(__FILE__, __LINE__, MSG)
#define ISX_THROW_EXCEPTION_DATAIO(MSG) throw isx::ExceptionDataIO(__FILE__, __LINE__, MSG)
#define ISX_THROW_EXCEPTION_USRINPUT(MSG) throw isx::ExceptionUserInput(__FILE__, __LINE__, MSG)

namespace isx
{
    /// The base class for exceptions
    ///
    class Exception : public std::exception
    {
    public:
        
        /// Default message for exceptions
        static const std::string DEFAULT_MSG;

        /// Constructor
        ///
        explicit Exception(const char* file, int line, const std::string& message) :
            m_fileName(file),
            m_msg(message)            
        {
            m_line = std::to_string(line);
            ISX_LOG_ERROR("Exception in line " + m_line + " of file " + m_fileName + " - " + m_msg);
        }

        /// Destructor
        ///
        virtual ~Exception() throw () {}

        /// Retrieves the error message
        virtual const char* what() const throw () 
        {
            return m_msg.c_str();
        }

        /// Retrieves the file name in which failure occurs
        ///
        const char* getFileName() const
        {
            return m_fileName.c_str();
        }
        
        /// Return the line where failure occurs
        ///
        int getLine()
        {
            return std::atoi(m_line.c_str());
        }

 
    protected:
        
        std::string m_msg;           //!< Detailed message
        std::string m_fileName;      //!< Failing file
        std::string m_line;          //!< Failing line within the file
     };

    /// File I/O exception class
    ///
    class ExceptionFileIO : public Exception
    {
    public:
        
        /// Constructor
        ///
        explicit ExceptionFileIO(const char* file, int line, const std::string& message = DEFAULT_MSG) :
            Exception(file, line, message)
        {}
        
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
        explicit ExceptionDataIO(const char* file, int line, const std::string& message = DEFAULT_MSG) :
            Exception(file, line, message)
        {}
        
        /// Destructor
        ///
        ~ExceptionDataIO() {}
    };
    
    /// User input exception class
    ///
    class ExceptionUserInput : public Exception
    {
    public:

        /// Constructor
        ///
        explicit ExceptionUserInput(const char* file, int line, const std::string& message = DEFAULT_MSG) :
            Exception(file, line, message)
        {}
        
        /// Destructor
        ///
        ~ExceptionUserInput() {}
    };
}


#endif //ISX_EXCEPTION_H
