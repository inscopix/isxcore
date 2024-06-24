#ifndef ISX_LOGGER_H
#define ISX_LOGGER_H


#include <memory>
#include <string> 

namespace isx {

/// A class implementing a singleton Logger to be used
/// application-wide for logging user actions, errors, debug info and warnings to a file.
///
class Logger
{
public:
    /// destructor
    ///
    ~Logger();

    /// Singleton initializer
    /// \param inLogFileName if empty, nothing will be logged
    static
    void
    initialize(const std::string & inLogFileName);

    /// Check if singleton has been initialized
    /// \return bool indicating the above
    ///
    static
    bool 
    isInitialized();

    /// \return pointer to the Logger singleton instance
    ///
    static
    Logger *
    instance();

    /// Add a line to the log
    /// \param text text to be logged
    static
    void
    log(const std::string & text);

    /// \return log filename
    ///
    static
    const std::string &
    getLogFileName();

private:
    Logger(const std::string & inLogFileName);
    Logger(const Logger & other) = delete;
    const Logger & operator=(const Logger & other) = delete;

    class Impl;
    std::shared_ptr<Impl> m_pImpl;
    
    static std::unique_ptr<Logger> s_instance;
};

} // namespace isx

#endif // def ISX_LOGGER_H