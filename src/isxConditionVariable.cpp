#include "isxConditionVariable.h"
#include "isxMutex.h"

#include <QMutex>
#include <QWaitCondition>

namespace isx
{

class ConditionVariable::Impl
{
public:
    Impl(){}
    ~Impl(){}
    
    void
    wait(Mutex & inMutex)
    {
        QMutex * m = static_cast<QMutex *>(inMutex.getNativeHandle());
        m_waitCondition.wait(m, ULONG_MAX);
    }

    bool
    waitForMs(Mutex & inMutex, uint32_t inWaitForMs)
    {
        QMutex * m = static_cast<QMutex *>(inMutex.getNativeHandle());
        return m_waitCondition.wait(m, inWaitForMs);
    }

    void
    notifyOne()
    {
        m_waitCondition.wakeOne();
    }

    void
    notifyAll()
    {
        m_waitCondition.wakeAll();
    }

private:
    QWaitCondition m_waitCondition;
};

ConditionVariable::ConditionVariable()
: m_pImpl(new Impl())
{}

ConditionVariable::~ConditionVariable() {}

void
ConditionVariable::wait(Mutex & inMutex)
{
    m_pImpl->wait(inMutex);
}

bool
ConditionVariable::waitForMs(Mutex & inMutex, uint32_t inWaitForMs)
{
    return m_pImpl->waitForMs(inMutex, inWaitForMs);
}

void
ConditionVariable::notifyOne()
{
    m_pImpl->notifyOne();
}

void
ConditionVariable::notifyAll()
{
    m_pImpl->notifyAll();
}

} // namespace isx
