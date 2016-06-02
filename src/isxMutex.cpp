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
    
    
    Mutex::Mutex()
    {
        m_internal.reset(new Impl());
    }

    Mutex::~Mutex()
    {
    }

    void Mutex::lock()
    {
        m_internal->lock();
    }

    void Mutex::unlock()
    {
         m_internal->unlock();
    }

}