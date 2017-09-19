#include "stdafx.h"
#include "ZipEx.h"


CZipEx::CZipEx()
{
}


CZipEx::~CZipEx()
{
}

void CZipEx::CreateZip(const TCHAR *fn, const char *password)
{
	m_Hzip = ::CreateZip(fn, password);
}

void CZipEx::ZipAdd(const TCHAR *dstzn, const TCHAR *fn)
{
	if (m_Hzip != NULL)
		::ZipAdd(m_Hzip, dstzn, fn);
}

void CZipEx::CloseZipEx()
{
	CloseZip(m_Hzip);
}