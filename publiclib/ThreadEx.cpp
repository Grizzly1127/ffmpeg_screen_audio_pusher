// ThreadEx.cpp: implementation of the ThreadEx class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ThreadEx.h"
#ifndef WIN32
#include <pthread.h>
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ThreadEx::ThreadEx()
{
#ifndef WIN32
	sem_init(&m_semWaitForStop,0,1);
	m_thread = -1;
	m_bThreadStopped=true;
#else
	m_dwThreadID=0;
	m_hThread=NULL;
	m_evStop = CreateEvent(NULL,TRUE,TRUE,NULL);
	SetEvent(m_evStop);
#endif
}

ThreadEx::~ThreadEx()
{
#ifndef WIN32
	sem_destroy(&m_semWaitForStop);
#else
	CloseHandle(m_evStop);
#endif
}

bool ThreadEx::StartThread()
{
#ifndef WIN32
	if (pthread_create(&m_thread,NULL,InitThreadProc,(void*)this)!=0)
		return false;
	sem_wait(&m_semWaitForStop);
	m_bThreadStopped=false;
	return true;
#else
	//如果线程已经创建，则不需要再创建
	if (!m_hThread){ 
		//创建并启动线程函数
		m_hThread = CreateThread(
					NULL,			//the handle cannot be inherited. 
                    0,				//the default Thread Stack Size is set
                    InitThreadProc,	//线程函数
                    this,			//线程函数的参数
                    0,				//使线程函数创建完后立即启动
                    &m_dwThreadID	//receives the thread identifier
					);
                
        }//end if (!m_hThread...

	if (m_hThread)
		ResetEvent(m_evStop);

	return	m_hThread != NULL;
#endif
}

void ThreadEx::WaitForStop()
{
#ifndef WIN32
	if (m_bThreadStopped==false)
	{
		sem_wait(&m_semWaitForStop);
		m_bThreadStopped=true;
		pthread_join(m_thread,NULL);
	}
#else
		
	WaitForSingleObject(m_evStop,INFINITE);

	// 返回线程句柄
	HANDLE hThread = (HANDLE)InterlockedExchange((LONG *)&m_hThread, 0);
	if (hThread) {
		// 等待线程终止
		WaitForSingleObject(hThread, INFINITE);
		// 关闭线程句柄
		CloseHandle(hThread);
	}// end if (hThread...
#endif
}

unsigned long  ThreadEx::ThreadProc()
{
 	ThreadProcMain();
#ifndef WIN32
	sem_post(&m_semWaitForStop);
#else
	SetEvent(m_evStop);
#endif
	return 0;
}

