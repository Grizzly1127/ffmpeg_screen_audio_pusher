#include "stdafx.h"
#include "ToolEx.h"


ToolEx::ToolEx()
{
}


ToolEx::~ToolEx()
{
}

bool ToolEx::AlreadyRunning(LPCWSTR strAppName)
{
	bool bFound = true;

	//看看能不能运行多个实例
	// Try to create a mutex with the app's name
	HANDLE hMutexOneInstance = ::CreateMutexW(NULL, TRUE, strAppName);

	// Already there...means that we are already running an instance
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		bFound = false;

	// Release the mutex
	if (hMutexOneInstance)
		::ReleaseMutex(hMutexOneInstance);

	return(bFound);
}