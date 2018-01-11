#ifndef _Mt_Mutex_
#define _Mt_Mutex_

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class MtMutex
{
public:
	MtMutex()
	{
#ifdef WIN32
		InitializeCriticalSectionAndSpinCount(&m_tCriticalSection, 
			0x80000400);
#else
		pthread_mutex_init(&m_tMutex, NULL);
#endif
	}

	~MtMutex()
	{
#ifdef WIN32
		// Release resources used by the critical section object.
		DeleteCriticalSection(&m_tCriticalSection);
#else
		pthread_mutex_destroy(&m_tMutex);
#endif
	}

	inline void lock()
	{
#ifdef WIN32
		// Request ownership of the critical section.
		EnterCriticalSection(&m_tCriticalSection);
#else
		pthread_mutex_lock(&m_tMutex);
#endif
	}

	inline void unLock()
	{
#ifdef WIN32
		// Release ownership of the critical section.
		LeaveCriticalSection(&m_tCriticalSection);
#else
		pthread_mutex_unlock(&m_tMutex);
#endif
	}

private:

#ifdef WIN32
	CRITICAL_SECTION m_tCriticalSection;
#else
	pthread_mutex_t m_tMutex;
#endif

};

class MTLock
{
public:
	inline MTLock(MtMutex* pMutex)
	{
		m_pMutex = pMutex;
		m_pMutex->lock();
	};

	inline ~MTLock()
	{
		m_pMutex->unLock();
	};

private:
	MtMutex* m_pMutex;
};

#endif
