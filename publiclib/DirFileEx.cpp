#include "stdafx.h"
#include "DirFileEx.h"
#include "LogEx.h"

static char g_szPathMulti[512] = { 0 };//当前模块工作路径
static TCHAR g_szPath[512] = { 0 }; //当前模块工作路径
static TCHAR g_szeClassLogPath[512] = { 0 }; //eclass log路径
static TCHAR g_szeClassDocumentPath[512] = { 0 }; //eclass document路径
static TCHAR g_szeClassScreenshotPath[512] = { 0 }; //eclass 屏幕截图保存路径
static TCHAR g_szeClassAppdataPath[MAX_PATH] = { 0 }; //eclass 录像设置保存路径

//功能：获取程序当前运行目录
//参数：无
//返回值：当前运行目录
const char* GetRunDir()
{
	if ('\0' == g_szPathMulti[0])
	{
		memset(g_szPathMulti, 0, sizeof(g_szPathMulti));
		GetModuleFileNameA(NULL, g_szPathMulti, sizeof(g_szPathMulti));
		char *pos = strrchr(g_szPathMulti, '\\');
		if (!pos)
		{
			return "";//不能用return NULL，外部调用strcpy会崩溃
		}
		*(pos + 1) = '\0';
	}

	return g_szPathMulti;
}

const TCHAR* GetRunDirW()
{
	//TCHAR szPath[512] = { 0 };
	if (_T('\0') == g_szPath[0])
	{
		memset(g_szPath, 0, sizeof(g_szPath));
		GetModuleFileName(NULL, g_szPath, sizeof(g_szPath)); //sizeof(g_szPath) == sizeof(TCHAR)* 512

		TCHAR *pos = wcsrchr(g_szPath, _T('\\')); //wcsrchr:从一个字符串中寻找某个字符最后出现的位置
		if (!pos)
		{
			return _T("");//不能用return NULL，外部调用strcpy会崩溃
		}
		*(pos + 1) = _T('\0');
	}

	return g_szPath;
}

/*********************************************************************
* 函数名称:bool Ex_CreateDiretory(const char* pszDir)
* 说明:创建目录
* 调用者：anyone
* 输入参数:
* const char *pszDir --用户指定的路径
* 输出参数：
* 无
* 返回值:
* true 表示成功，flase 表示失败
* 注意：
*********************************************************************/
bool Ex_CreateDiretory(const char* pszDir)
{
	if (!pszDir)
	{
		return false;
	}

	//如果不包含\，则在当前目录下创建
	if (strstr(pszDir, "\\") == NULL)
	{
		//获取当前路径+需要创建的文件夹名称
		char szPathAll[512] = { 0 };
		sprintf(szPathAll, "%s%s", GetRunDir(), pszDir);

		int iRet = CreateDirectoryA(szPathAll, NULL);
		if (0 == iRet)
		{
			iRet = GetLastError();
			if (ERROR_ALREADY_EXISTS == iRet)
			{
				return true;
			}
			else
				return false;
		}
	}
	else//全路径，并且可能有多级目录，后面的目录可能没有创建
	{
		int iRet = 0;
		char szSub[512] = { 0 };
		const char *pSub = NULL;
		int iIndex = 0;
		int iLen = 0;
		int bFind = 0;

		//逐层创建目录
		while (true)
		{
			pSub = strchr(pszDir + iLen, '\\');
			if (NULL == pSub)//如果找不到'\'
			{
				//如果是根目录,比如'C:\\'
				if (iLen == 0)
				{
					return false;
				}
				//如果已经创建到最后一级目录
				iRet = CreateDirectoryA(pszDir, NULL);
				if (0 == iRet)
				{
					iRet = GetLastError();
					if (ERROR_ALREADY_EXISTS == iRet)
					{
						return true;
					}
					else
						return false;
				}
			}//if (NULL == pSub)//如果找不到'\'
			else//找到'\'
			{
				if (!bFind)//跳过第一个'\'
				{
					bFind = 1;
				}
				else
				{
					memset(szSub, 0, sizeof(szSub));
					strncpy(szSub, pszDir, pSub - pszDir);
					CreateDirectoryA(szSub, NULL);
				}
				iLen = pSub - pszDir + 1;
			}//else//找到'\'
		}//while (true)
	}//else//全路径，并且可能有多级目录，后面的目录可能没有创建
	return true;
}

bool  Ex_CreateDiretory(const TCHAR* pszDir)
{
	CharCode charPath(pszDir);
	return Ex_CreateDiretory(charPath.GetStringA());
}

//检查文件是否存在
bool IsFileExist(const TCHAR* pszFileName)
{
	if (!pszFileName)
	{
		return false;
	}
#ifdef _UNICODE
	if (GetFileAttributesW(pszFileName) == -1)
#else
	if (GetFileAttributesA(pszFileName) == -1)
#endif
	{
		return false;
	}

	return true;
}

//检查目录是否存在
bool IsDirectoryExist(const char* psDirName)
{
	return false;
}

//检查路径是否存在，如何不存在则创建
bool CheckPathCreate(const char* pszPath)
{
	//char*t = file_name;
	//while (t = strchr(++t, '\\'))
	//{		
	//	*t = 0;		
	//	if (access(file_name, 0) != -1) 
	//	{
	//		*t = '\\';
	//		continue;
	//	}		
	//	mkdir(file_name);		
	//	*t = '\\';
	//}
	return false;
}

int GetFileLen(TCHAR* szFileName)
{
	CFile hFile;
	int nLength = 0;
	
	if (!(hFile.Open(szFileName, CFile::modeRead)))
	{
		return 0;
	}
	nLength = hFile.GetLength();
	hFile.Close();
	return nLength;
}

//功能：检查有没有该文件夹，没有则创建
//参数：sFolder:文件夹的全路径
//返回值：无
//void CheckFolder(CString sFolder)
//{
//	sFolder = sPath + "\\" + sFolder;
//	CFileFind finder;
//	if(!(finder.FindFile(sFolder)))		
//		CreateDirectory(sFolder,NULL);
//	finder.Close();
//}


//AFX_EXT_CLASS void GetAllFileName(CString&		strAllFileName,
//								  LPCTSTR	cszDirectory)
//{
//	WIN32_FIND_DATA FindFileData;
//	HANDLE hFind;
//
//	hFind = FindFirstFile(cszDirectory, &FindFileData);
//	strAllFileName = _T("");
//
//	if (hFind == INVALID_HANDLE_VALUE)
//	{
//		if(hFind)
//			FindClose(hFind);
//		strAllFileName = _T("");
//	} 
//	else 
//	{
//		CString strTempComp = _T("");
//		do 
//		{
//			strTempComp = FindFileData.cFileName;
//			if (strAllFileName.Compare(_T("")))
//			{
//				strAllFileName = strAllFileName + "," + strTempComp;
//			}
//			else
//			{
//				strAllFileName = strTempComp;
//			}
//		}while (FindNextFile(hFind,&FindFileData));
//	}
//	if(hFind)
//		FindClose(hFind);
//}

//AFX_EXT_CLASS void RemoveOldDirectory(LPCTSTR strOldDirectory)
//{
//	RemoveDirectory(strOldDirectory);
//}


//获取当前目录是否可写
BOOL GetRunDirWriten()
{
	BOOL bWriteble = FALSE;
	CFile   file;
	CString cFilePath = _T("tmp.txt");
	Write_LogW(LOG_INFO, _T("DirFileEx: Start GetRunDirWriten..."));
	if (file.Open(cFilePath.GetBuffer(0), CFile::modeCreate | CFile::modeWrite))
	{
		bWriteble = TRUE;
		file.Close();
		file.Remove(cFilePath.GetBuffer(0));
	}
	Write_LogW(LOG_INFO, _T("DirFileEx: GetRunDirWriten:%s."), bWriteble ? _T("true") : _T("false"));

	return bWriteble;
}

//Log和document都放在tmp目录下
const TCHAR* GetLogDir()
{
	if (!g_szeClassLogPath[0])
	{
		TCHAR szTempPath[256] = { 0 };
		DWORD dwSize = 128;
		GetTempPath(dwSize, szTempPath);

		CString cstrTmpPath = szTempPath;
		cstrTmpPath += _T("eclass\\log_eClass\\");

		CharCode szDir(cstrTmpPath.GetBuffer(0));
		if (!Ex_CreateDiretory(szDir.GetStringA()))
		{
			CString cstrLogPath = GetRunDirW();
			cstrLogPath += _T("\\log_eClass\\");
			wcscpy(g_szeClassLogPath, cstrLogPath.GetBuffer(0));
		}
		else
		{
			wcscpy(g_szeClassLogPath, cstrTmpPath.GetBuffer(0));
		}
	}

	return g_szeClassLogPath;
}

//Log和document都放在tmp目录下
const TCHAR* GetDocumentDir()
{
	if (!g_szeClassDocumentPath[0])
	{
		TCHAR szTempPath[256] = { 0 };
		DWORD dwSize = 128;
		GetTempPath(dwSize, szTempPath);

		CString cstrTmpPath = szTempPath;
		cstrTmpPath += _T("eclass\\document\\");

		CharCode szDir(cstrTmpPath.GetBuffer(0));
		if (!Ex_CreateDiretory(szDir.GetStringA()))
		{
			Write_LogW(LOG_ERROR, _T("DirFileEx: Ex_CreateDiretory error:%s."), cstrTmpPath.GetBuffer(0));
			CString cstrDocPath = GetRunDirW();
			cstrDocPath += _T("\\document\\");
			wcscpy(g_szeClassDocumentPath, cstrDocPath.GetBuffer(0));
		}
		else
		{
			wcscpy(g_szeClassDocumentPath, cstrTmpPath.GetBuffer(0));
		}
	}

	return g_szeClassDocumentPath;
}

const TCHAR* GetScreenshotDir()
{
	if (!g_szeClassScreenshotPath[0])
	{
		TCHAR szTempPath[256] = { 0 };
		DWORD dwSize = 128;
		GetTempPath(dwSize, szTempPath);

		CString cstrTmpPath = szTempPath;
		cstrTmpPath += _T("eclass\\screenshot\\");

		CharCode szDir(cstrTmpPath.GetBuffer(0));
		if (!Ex_CreateDiretory(szDir.GetStringA()))
		{
			Write_LogW(LOG_ERROR, _T("DirFileEx: Ex_CreateDiretory error:%s."), cstrTmpPath.GetBuffer(0));
			CString cstrDocPath = GetRunDirW();
			cstrDocPath += _T("\\screenshot\\");
			wcscpy(g_szeClassScreenshotPath, cstrDocPath.GetBuffer(0));
		}
		else
		{
			wcscpy(g_szeClassScreenshotPath, cstrTmpPath.GetBuffer(0));
		}
	}

	return g_szeClassScreenshotPath;
}

const TCHAR* GetAppdataDir()
{
	if (!g_szeClassAppdataPath[0])
	{
		SHGetSpecialFolderPath(NULL, g_szeClassAppdataPath, CSIDL_APPDATA, TRUE);
	}
	return g_szeClassAppdataPath;
}