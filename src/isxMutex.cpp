#include "isxMutex.h"
#include "isxMutex_internal.h"

namespace isx
{
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