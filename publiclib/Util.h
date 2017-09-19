#ifndef __UTIL_h__
#define __UTIL_h__

#include "def.h"

EXPORT_FUNCTION unsigned long Ex_GetTickCount(void);
EXPORT_FUNCTION unsigned long GenerateSSRC(void);
EXPORT_FUNCTION void EX_Sleep(unsigned long ulMS);
EXPORT_FUNCTION void EX_Delay(unsigned long ulMS);
//#define StringToULONG(string) strtoul(string,NULL,10)

BOOL KillProcessFromName(CString strProcessName);
CString GetOSVersoin(int& nHVersion, int& nLVersion);
BOOL RegisterUrlProtecl(const char* cszPathAll);

 unsigned __int64 RDTSC();
 //获取CPU的主频
 EXPORT_FUNCTION double CPUClockSpeed();

CRect EXPORT_FUNCTION AfxGetDesktopRect();
CRect EXPORT_FUNCTION AfxGetMaxWindowClientRect();

EXPORT_FUNCTION int StringEncrypt(char* pKey, char* pText, unsigned char* pResult);//字符串加密
EXPORT_FUNCTION bool StringDecrypt(char* pKey, unsigned char* pDate, int nDataLen, char*pResult);//字符串解密
#endif