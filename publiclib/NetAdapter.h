#ifndef __NET_ADAPTER_H__
#define __NET_ADAPTER_H__


#include "def.h"

EXPORT_FUNCTION int GetNetAdapterType(void);
EXPORT_FUNCTION int GetHostIpByDomain(CString cstrDomain, CString &cstrIp);
#endif