// AutoLock.h: interface for the AutoLock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __AUTOLOCK_H__
#define __AUTOLOCK_H__

#include "def.h"

#include "CritSec.h"
class EXPORT_FUNCTION AutoLock
{
public:
	AutoLock(CritSec*pAVCritSec);
	~AutoLock();
protected:
    CritSec * m_pAVCritSec;
};

#endif 