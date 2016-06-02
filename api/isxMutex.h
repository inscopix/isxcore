#ifndef ISX_MUTEX_H
#define ISX_MUTEX_H

#include "isxObject.h"

#include <memory>
#include <string>
#include <thread>

namespace isx
{
    ///
    /// A class implementing a Mutex.
    ///

    class Mutex : public Object
    {
    public:
        // Methods 

        /// Default constructor
        ///
        Mutex();

        /// Default Destructor
        ///
        ~Mutex();

        /// lock mutex
        /// \param inOwner string identifying the owning code
        ///
        void lock(const std::string & inOwner);

        /// unlock mutex
        ///
        void unlock();

        /// Serialize the object into an output stream.
        ///
        /// \param   strm    The output stream.
        virtual void serialize(std::ostream& strm) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_internal;

        std::string m_owner;
        std::thread::id m_owningThread;
    };

    /// A class implementing RAII for mutexes
    ///
    class ScopedMutex
    {
    public:
        /// Constructor
        /// \param inMutex mutex to lock 
        /// \param inOwner string identifying the owning code
        ScopedMutex(Mutex & inMutex, const std::string & inOwner);

        /// Destructor
        ~ScopedMutex();
    private:
        Mutex & m_mutex;   
    };
}

#endif // ISX_MUTEX_H
