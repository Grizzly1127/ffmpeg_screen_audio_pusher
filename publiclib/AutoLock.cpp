// AutoLock.cpp: implementation of the AutoLock class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AutoLock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
AutoLock::AutoLock(CritSec*pAVCritSec)
{
	if (pAVCritSec)
	{
		m_pAVCritSec = pAVCritSec;
		m_pAVCritSec->Lock();
	}
	
}

AutoLock::~AutoLock()
{
	if (m_pAVCritSec)
		m_pAVCritSec->Unlock();
}
