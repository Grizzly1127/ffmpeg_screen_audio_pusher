/************************************
*comment:日期、时间相关函数
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __DIRFILE_EX_H__
#define __DIRFILE_EX_H__


#include "def.h"
#include "CharCode.h"


EXPORT_FUNCTION const char* GetRunDir();
EXPORT_FUNCTION const TCHAR* GetRunDirW();
EXPORT_FUNCTION bool  Ex_CreateDiretory(const char* pszDir);
EXPORT_FUNCTION bool  Ex_CreateDiretory(const TCHAR* pszDir);
EXPORT_FUNCTION bool  IsFileExist(const TCHAR* pszFileName);
EXPORT_FUNCTION bool  IsDirectoryExist(const char* psDirName);
EXPORT_FUNCTION bool  CheckPathCreate(const char* pszPath);
EXPORT_FUNCTION int  GetFileLen(TCHAR* szFileName);
EXPORT_FUNCTION BOOL GetRunDirWriten();
EXPORT_FUNCTION const TCHAR* GetLogDir();
EXPORT_FUNCTION const TCHAR* GetDocumentDir();
EXPORT_FUNCTION const TCHAR* GetScreenshotDir();
EXPORT_FUNCTION const TCHAR* GetAppdataDir();

#endif