/************************************
*comment:日志输出到文件
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __INI_EX_H__
#define __INI_EX_H__

#include "def.h"

EXPORT_FUNCTION void WriteIniInt(LPCTSTR lpcszSec, LPCTSTR lpcszKey, int nValue);
EXPORT_FUNCTION void WriteIniString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, LPCTSTR lpcszValue);
EXPORT_FUNCTION int ReadIniInt(LPCTSTR lpcszSec, LPCTSTR lpcszKey, int nDefValue);
EXPORT_FUNCTION void ReadIniString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, LPCTSTR lpcszDefValue, TCHAR* lpValue, int nMax);
EXPORT_FUNCTION void LoadLanguageString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, TCHAR* lpValue, int nMax);



#endif