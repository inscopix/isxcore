#ifndef ISX_CONDITION_VARIABLE_H
#define ISX_CONDITION_VARIABLE_H

#include <memory>

namespace isx
{

/// forward declare Mutex class (isxMutex.h)
class Mutex;

///
/// A class implementing a Condition Variable.
/// Modeled after std::condition_variable
///

class ConditionVariable
{
    typedef void * NativeMutex_t;

public:
    /// Default constructor
    ///
    ConditionVariable();

    /// Delete copy constructor
    ///
    ConditionVariable(const ConditionVariable &) = delete;

    /// Default Destructor
    ///
    ~ConditionVariable();

    /// Blocks the current thread until the condition variable is woken up
    /// \param inMutex mutex to use, must be locked by current thread
    ///
    void
    wait(Mutex & inMutex);

    /// Blocks the current thread until the condition variable is woken up 
    /// or after the specified timeout duration
    /// \param inMutex mutex to use, must be locked by current thread
    /// \param inWaitForMs the maximum time to spend waiting
    /// \return false if the wait timed out
    ///
    bool
    waitForMs(Mutex & inMutex, uint32_t inWaitForMs);

    /// notifies one waiting thread
    ///
    void
    notifyOne();

    /// notifies all waiting threads
    ///
    void
    notifyAll();

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};
} // namespace isx

#endif // def ISX_CONDITION_VARIABLE_H
