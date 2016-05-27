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
        
    private:
        QMutex m_mutex;

    };
    
    
    
    Mutex::Mutex() :
        m_bIsLocked(false)
    {
        m_internal.reset(new Impl());
    }

    Mutex::~Mutex()
    {
    }

    void Mutex::lock()
    {
        m_internal->lock();
        m_bIsLocked = true;
    }

    void Mutex::unlock()
    {
        if (m_bIsLocked)
        {
            m_internal->unlock();
            m_bIsLocked = false;
        }
    }

}