#ifndef ISX_MUTEX_H
#define ISX_MUTEX_H

#include <memory>

namespace isx
{
    ///
    /// A class implementing a Mutex.
    ///

    class Mutex
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
        ///
        void lock();

        /// unlock mutex
        ///
        void unlock();

    private:
        class Impl;
        std::unique_ptr<Impl> m_internal;
    };
}

#endif // ISX_MUTEX_H
