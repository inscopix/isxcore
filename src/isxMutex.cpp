#include "isxMutex.h"

#include <QMutex>


namespace isx
{
    /// A wrapper around QMutex
    ///
    class Mutex::Impl
    {
    public: 
        /// Constructor
        ///
        Impl(){}

        /// Destructor
        ///
        ~Impl(){}
        
        void lock() { m_mutex.lock();}
        void unlock() { m_mutex.unlock();}

        void * getNativeHandle() { return &m_mutex; }
        
    private:
        QMutex m_mutex;
    };
    
    
    Mutex::Mutex()
    {
        m_internal.reset(new Impl());
    }

    Mutex::~Mutex()
    {
    }

    void Mutex::lock(const std::string & inOwner)
    {
        m_internal->lock();
        m_owner = inOwner;
        m_owningThread = std::this_thread::get_id();
    }

    void Mutex::unlock()
    {
        m_owner = std::string();
        m_internal->unlock();
    }

    void * Mutex::getNativeHandle()
    {
        return m_internal->getNativeHandle();
    }

    void Mutex::serialize(std::ostream& strm) const
    {
        strm << "Owner: " << m_owningThread << ", " << m_owner;
    }

    ScopedMutex::ScopedMutex(Mutex & inMutex, const std::string & inOwner)
    : m_mutex(inMutex)
    {
        m_mutex.lock(inOwner);
    }

    ScopedMutex::~ScopedMutex()
    {
        m_mutex.unlock();
    }
}