#include "stdafx.h"
#include "UnicodeToUtf8.h"


UnicodeToUtf8::UnicodeToUtf8(const char* pszSrc)
: m_pszDes(NULL)
{
	int iLenWide = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	TCHAR* szData = new TCHAR[iLenWide];
	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, szData, iLenWide);

	int nLen = wcslen(szData);
	ConvertToUtf8(szData, nLen);

	if (szData)
	{
		delete[] szData;
		szData = NULL;
	}
}

UnicodeToUtf8::UnicodeToUtf8(const TCHAR* pszSrc)
: m_pszDes(NULL)
{
	int nLen = wcslen(pszSrc);

	ConvertToUtf8(pszSrc, nLen);
}


UnicodeToUtf8::~UnicodeToUtf8()
{
	if (m_pszDes)
	{
		delete[] m_pszDes;
		m_pszDes = NULL;
	}
}

//·µ»Øutf-8±àÂë
const char* UnicodeToUtf8::GetString()
{
	return m_pszDes;
}

void UnicodeToUtf8::ConvertToUtf8(const TCHAR* pszSrc, int nLen)
{
	m_pszDes = new char[nLen * 3 + 3];
	memset(m_pszDes, 0x00, nLen * 3 + 3);
	strcpy(m_pszDes, CW2A(pszSrc, CP_UTF8));
}
