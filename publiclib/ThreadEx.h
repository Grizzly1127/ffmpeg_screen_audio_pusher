#ifndef __THREAD_EX_H__
#define __THREAD_EX_H__

#ifndef WIN32
#define _PTHREAD
#endif

#include "def.h"

class EXPORT_FUNCTION ThreadEx
{
public:
	ThreadEx();
	virtual ~ThreadEx();
public:
	/*
	启动线程，成功返回true,失败返回false,只有在该进程创建的线程数到达系统限制时才返回false
	*/
	bool StartThread(void);
	/*
	等待线程退出，调用此函数前必须调用方法，以确保从主线程函数中退出，否则调用此函数会阻塞
	*/
	void WaitForStop(void);

	/*
	此函数只供类内部调用
	*/
#ifdef WIN32
	static DWORD WINAPI InitThreadProc(PVOID pObj){
		return	((ThreadEx*)pObj)->ThreadProc();
	}
#else
	static void* InitThreadProc(void*pObj){
		((ThreadEx*)pObj)->ThreadProc();
		return 0;
	}
#endif
	/*
	此函数只供类内部调用
	*/
	unsigned long ThreadProc(void);

protected:
	/*
	主线程函数，您可以在此函数中实现线程要做的事
	*/
	virtual void ThreadProcMain(void)=0;
protected:
#ifdef WIN32
	DWORD	m_dwThreadID;		// 线程标识
	HANDLE	m_hThread;			// 线程句柄
	HANDLE	m_evStop;
#else
    pthread_t	m_thread;
    sem_t		m_semWaitForStop;
    bool		m_bThreadStopped;
#endif
};

/*
线程用例

class TestThread:public ThreadEx
{
public:
	TestThread()
	{
		m_bWantToStop=false;
	};
	virtual~TestThread()
	{
	};
public:
	bool Connect(void)
	{
		//启动线程
		return StartThread();
	};

	void ReleaseConnections(void)
	{
		//从主线程函数中退出
		m_bWantToStop=true;
		//等待线程退出
		WaitForStop();
	};
protected:
	void ThreadProcMain(void)
	{
		//每隔一秒输出一个数字，从0开始记数
		int i=0;
		//只要没有退出信号就继续输出
		while(!m_bWantToStop)
		{
			TRACE("i=%d\n",i++);
			Sleep(1000);
		}
	};
protected:
	bool	m_bWantToStop;
};
*/

#endif
