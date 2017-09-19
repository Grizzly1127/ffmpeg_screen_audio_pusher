#pragma once
#ifndef __ZIPEX_H__
#define __ZIPEX_H__

#include "zip.h"
#include "def.h"

class EXPORT_FUNCTION CZipEx
{
public:
	CZipEx();
	~CZipEx();

	void CreateZip(const TCHAR *fn, const char *password);
	void ZipAdd(const TCHAR *dstzn, const TCHAR *fn);
	void CloseZipEx();
private:
	HZIP m_Hzip;
};

#endif