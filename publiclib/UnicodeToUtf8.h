/************************************
*comment:unicode×ªutf-8×Ö·û±àÂë×ª»»
*auth:lihuixiong
*date:2016.8.12
*************************************/

#ifndef __UNICODE_TO_UTF8_H__
#define __UNICODE_TO_UTF8_H__

#include "def.h"

class EXPORT_FUNCTION UnicodeToUtf8
{
public:
	UnicodeToUtf8(const char* pszSrc);
	UnicodeToUtf8(const TCHAR* pszSrc);
	~UnicodeToUtf8();

	//·µ»Øutf-8±àÂë
	const char* GetString();

private:
	char* m_pszDes;

	void ConvertToUtf8(const TCHAR* pszSrc, int nLen);
};

#endif