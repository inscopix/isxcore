#include "isxLogger.h"
#include "isxTime.h"
#include "isxPathUtils.h"


#include <fstream>
#include <cstdio>
#include <QString>
#include <QDir>

namespace isx {

std::unique_ptr<Logger> Logger::s_instance;

class Logger::Impl : public std::enable_shared_from_this<Logger::Impl>
{
    typedef std::weak_ptr<Impl>     WpImpl_t;
    typedef std::shared_ptr<Impl>   SpImpl_t;

public:
    Impl(const std::string & inLogFileName)
    {
        //QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        m_filename = inLogFileName;
        std::string path = getDirName(m_filename);

        // Ensure the path exists
        QDir dir(QString::fromStdString(path));
        if (false == dir.exists())
        {
            dir.mkpath(QString::fromStdString(path));
        }

        std::remove(m_filename.c_str());
    }

    void log(const std::string & inText)
    {
        /// Note, there are no checks here (i.e. good()) on purpose. 
        /// Even if we get a bad stream, we can't "log" it and I don't think we 
        /// we should throw an exception (which also tries to log). 
        /// By default, streams don't throw exceptions so in the worst case-scenario 
        /// it will fail silently.
        std::ofstream file(m_filename, std::ios::out | std::ios::app);
        file << inText;
        file.flush();
        file.close();
    }

    
private: 

    std::string                         m_filename;

};

Logger::Logger(const std::string & inLogFileName)
{
    m_pImpl.reset(new Impl(inLogFileName));
}

Logger::~Logger()
{
}

void
Logger::initialize(const std::string & inLogFileName)
{
    if(inLogFileName.empty())
    {
        return;
    }

    if (!isInitialized())
    {
        s_instance.reset(new Logger(inLogFileName));
        const std::string str("Initialized session\n");
        s_instance->log(str);
    }
}

bool 
Logger::isInitialized()
{
    return (s_instance != nullptr);
}

Logger *
Logger::instance()
{
    if (isInitialized())
    {
        return s_instance.get();
    }
    return nullptr;
}

void

Logger::log(const std::string & text)
{
    if (isInitialized())
    {
        Time now = Time::now();
        const std::string strNow = now.toString();
        const std::string total = strNow + ": " + text;

        m_pImpl->log(total);
    }
}

}