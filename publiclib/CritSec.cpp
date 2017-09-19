// CritSec.cpp: implementation of the CritSec class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CritSec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CritSec::CritSec(void)
{
#ifdef WIN32
    InitializeCriticalSection(&m_CritSec);
#else
#if(0)
	//linux 7.3
	pthread_mutex_init(&m_pMutex, NULL);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_pMutex, &attr);
	pthread_mutexattr_destroy(&attr);
#endif
#endif
}

CritSec::~CritSec(void)
{
#ifdef WIN32
    DeleteCriticalSection(&m_CritSec);
#else
	pthread_mutex_destroy(&m_pMutex);
#endif
}

void CritSec::Lock(void)
{
#ifdef WIN32
    EnterCriticalSection(&m_CritSec);
#else
	(void)pthread_mutex_lock(&m_pMutex);
#endif
}

void CritSec::Unlock(void)
{
#ifdef WIN32
    LeaveCriticalSection(&m_CritSec);
#else
	pthread_mutex_unlock(&m_pMutex);
#endif
}
