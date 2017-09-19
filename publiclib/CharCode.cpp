/************************************
*comment:GBK和unicode字符编码转换
*auth:huangyubin
*date:2015.12.30
*************************************/

#include "stdafx.h"
#include "CharCode.h"

CharCode::CharCode(const char* pszSrc)
: m_pszDes(NULL)
, m_pwcharDes(NULL)
{
	if (!pszSrc)
	{
		return;
	}
	int nLen = strlen(pszSrc);

	if (nLen<=0)
	{
		return;
	}
	m_pszDes = new char[nLen + 1];

	if (!m_pszDes)
	{
		return;
	}
	memset(m_pszDes, 0x00, sizeof(char)*(nLen + 1));
	strcpy(m_pszDes, pszSrc);

	//从gbk转换为unicode
	MuitToWide(m_pszDes);
}


CharCode::CharCode(const TCHAR* pszSrc)
: m_pszDes(NULL)
, m_pwcharDes(NULL)
{
	if (!pszSrc)
	{
		return;
	}
	int nLen = wcslen(pszSrc);

	if (nLen <= 0)
	{
		return;
	}
	m_pwcharDes = new TCHAR[nLen + 1];

	if (!m_pwcharDes)
	{
		return;
	}
	memset(m_pwcharDes, 0x00, sizeof(TCHAR)*(nLen + 1));
	wcscpy(m_pwcharDes, pszSrc);
	//从unicode转换为gbk
	WideToMuit(m_pwcharDes);
}


CharCode::~CharCode()
{
	if (m_pszDes)
	{
		delete[] m_pszDes;
		m_pszDes = NULL;
	}

	if (m_pwcharDes)
	{
		delete[] m_pwcharDes;
		m_pwcharDes = NULL;
	}
}



char* CharCode::WideToMuit(TCHAR* ptcSrc)
{
	LPWSTR pwszStr = (LPWSTR)ptcSrc;
	int nDstStr = WideCharToMultiByte(CP_ACP, 0, pwszStr, -1, 0, 0, 0, 0);
	if (nDstStr <= 0)
	{
		return NULL;
	}
	m_pszDes = new char[nDstStr + 3];
	memset(m_pszDes, 0x00, sizeof(nDstStr + 3));
	int nDesLen = WideCharToMultiByte(CP_ACP, 0, pwszStr, -1, m_pszDes, nDstStr, 0, 0);

	if (nDesLen <= 0)
	{
		return NULL;
	}

	return m_pszDes;
}

//utf8显示
//char *lpszWord = new char[1024];
////读取后放进lpszWord 
//CString str = CA2W(lpszWord, CP_UTF8);

TCHAR* CharCode::MuitToWide(char* pSrc)
{
	//TCHAR* pWideString = NULL;//宽字符串指针
	int iLenWide = MultiByteToWideChar(CP_ACP, 0, pSrc, -1, NULL, 0);//获取窄字符串字符长度
	if (iLenWide <= 0)
	{
		return NULL;
	}

	m_pwcharDes = new wchar_t[iLenWide + 3];
	memset(m_pwcharDes, 0x00, sizeof(wchar_t)*(iLenWide + 3));
	if (!m_pwcharDes)
		return NULL;

	iLenWide = MultiByteToWideChar(CP_ACP, 0, pSrc, -1, m_pwcharDes, iLenWide);//窄字符转换成宽字符
	if (iLenWide <= 0)
	{
		return NULL;
	}
	
	return m_pwcharDes;
}


const char* CharCode::GetStringA()
{
	if (m_pszDes == NULL)
	{
		return "";
	}
	return m_pszDes;
}


const TCHAR* CharCode::GetStringW()
{
	if (m_pwcharDes == NULL)
	{
		return _T("");
	}
	return m_pwcharDes;
}