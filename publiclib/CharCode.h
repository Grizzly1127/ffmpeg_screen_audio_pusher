/************************************
*comment:GBKºÍunicode×Ö·û±àÂë×ª»» 
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __STRING_CODE_CONVERT_H__
#define __STRING_CODE_CONVERT_H__

#include "def.h"

class EXPORT_FUNCTION CharCode
{
public:
	CharCode(const char* pszSrc);
	CharCode(const TCHAR* pszSrc);
	~CharCode();

	//·µ»Øgbk±àÂë
	const char* GetStringA();
	//·µ»Øunicode±àÂë
	const TCHAR* GetStringW();
	

private:
	char* m_pszDes;
	TCHAR* m_pwcharDes;

	char* WideToMuit(TCHAR* ptcSrc);
	TCHAR* MuitToWide(char* pSrc);

};

#endif