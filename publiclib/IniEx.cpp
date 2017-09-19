#include "stdafx.h"
#include "IniEx.h"
#include "DirFileEx.h"

TCHAR g_szConfigPath[256] = { 0 };//
TCHAR g_szLangagePath[256] = { 0 };

class IniInit
{
public:
	IniInit();
	~IniInit();
};

IniInit::IniInit()
{
	wsprintf(g_szConfigPath, _T("%ssetting.ini"), GetRunDirW());

	//目前只支持中英两个版本
	LANGID lid = GetSystemDefaultLangID();
	if (lid == 0x0804)//简体中文
		wsprintf(g_szLangagePath, _T("%slang_sc.ini"), GetRunDirW());
	else
		wsprintf(g_szLangagePath, _T("%slang_en.ini"), GetRunDirW());
}

IniInit::~IniInit()
{
}
static IniInit g_IniInit;


void WriteIniInt(LPCTSTR lpcszSec, LPCTSTR lpcszKey, int nValue)
{
	TCHAR szValue[128] = { 0 };
#ifdef _UNICODE
	swprintf(szValue, _T("%d"), nValue);
#else
	sprintf(szValue, "%d", nValue);
#endif
	::WritePrivateProfileString(lpcszSec, lpcszKey, szValue, g_szConfigPath);
}
void WriteIniString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, LPCTSTR lpcszValue)  //app, keyname, keyvalue
{
	::WritePrivateProfileString(lpcszSec, lpcszKey, lpcszValue, g_szConfigPath);
}
int ReadIniInt(LPCTSTR lpcszSec, LPCTSTR lpcszKey, int nDefValue)
{
	return ::GetPrivateProfileInt(lpcszSec, lpcszKey, nDefValue, g_szConfigPath);
}
void ReadIniString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, LPCTSTR lpcszDefValue, TCHAR* lpValue, int nMax)
{
	::GetPrivateProfileString(lpcszSec, lpcszKey, lpcszDefValue, lpValue, nMax, g_szConfigPath);
}

void LoadLanguageString(LPCTSTR lpcszSec, LPCTSTR lpcszKey, TCHAR* lpValue, int nMax)
{
	if (!lpcszSec || !lpcszKey || !lpValue || nMax == 0)
		return;

	TCHAR szValue[1024] = { 0 };
	::GetPrivateProfileString(lpcszSec, lpcszKey, _T(""), szValue, nMax, g_szLangagePath);
	int nLen = wcslen(szValue);
	if (nLen < 1024)
		wcscpy(lpValue, szValue);
	else
		wcsncpy(lpValue, szValue, 1023);
}