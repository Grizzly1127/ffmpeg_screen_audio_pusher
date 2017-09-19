
#ifndef __HTTP_DOWNLOADEX_H__
#define __HTTP_DOWNLOADEX_H__

#include "ThreadEx.h"
#include "CHTTPDownload.h"
#include "def.h"

typedef enum enumErrorCode
{

};

class EXPORT_FUNCTION CHttpDownloadEx : public ThreadEx
{
public:
	CHttpDownloadEx();
	~CHttpDownloadEx();

protected:
	void ThreadProcMain(void);

public:
	bool HttpMutilDownload(TCHAR* szUrl, TCHAR* szDomain, TCHAR *szSaveFile);
	static bool HttpSingleDownload(CString strFileNameAll, CString strUrl);
	static bool HttpSingleDownloadByInternet(CString strFileNameAll, CString strUrl);
	static bool HttpSingleDownloadBySock(CString strFileNameAll, CString strUrl);
	void StopDownThread();
	ULONG GetDownloadBytes();

public:
	BOOL m_bSpeedTest;
private:
	CString m_strUrl;
	CString m_strDomain;
	CString m_strSaveFile;
	BOOL	m_bResult;
	CHTTPDownload m_http;
	
};

DWORD WINAPI DownloadThreadEx(LPVOID lpParam);

#endif