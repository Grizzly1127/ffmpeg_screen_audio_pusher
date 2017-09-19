// CritSec.h: interface for the CritSec class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CRITSEC_H__
#define __CRITSEC_H__

#include "def.h"

class EXPORT_FUNCTION CritSec
{
public:
	CritSec();
	virtual ~CritSec();
public:
    void Lock(void);
    void Unlock(void);
protected:
#ifdef WIN32
    CRITICAL_SECTION m_CritSec;
#else
	pthread_mutex_t		m_pMutex;
#endif
};

#endif 