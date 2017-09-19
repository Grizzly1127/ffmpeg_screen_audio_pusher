//Download by http://www.NewXing.com
// HTTPDownload.h: interface for the CHTTPDownload class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDOWNLOAD_H__BA195238_6DC5_4491_9173_FEC73B994BDC__INCLUDED_)
#define AFX_HTTPDOWNLOAD_H__BA195238_6DC5_4491_9173_FEC73B994BDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include <afxmt.h>

#include <afxsock.h>
#pragma comment(lib,"ws2_32.lib")
#include "CritSec.h"
#include "AutoLock.h"

//This struct is used to save the uncomplished task's info
//Now this program can only start a task once a time.
typedef struct tagDOWNLOADSTATE{
	CString url;
	CString localfile;
	CTime   time;
	LONG	length;
	LONG	range[8];
}DOWNLOADSTATE,*LPDOWNLOADSTATE;

#define DEFAULT_HTTP_PORT  80

// HTTP STATUS CODE分类
const UINT	HTTP_OK			= 0;
const UINT	HTTP_ERROR		= 1;
const UINT	HTTP_REDIRECT	= 2;
const UINT	HTTP_FAIL		= 3;

// 发送请求
const UINT SENDREQUEST_SUCCESS	= 0; // 成功
const UINT SENDREQUEST_ERROR	= 1; // 一般网络错误，可以重试
const UINT SENDREQUEST_STOP		= 2; // 中途停止(用户中断) (不用重试)
const UINT SENDREQUEST_FAIL		= 3; // 失败 (不用重试)	 

class CHTTPDownload  
{
public:
	UINT ThreadFunc(int index);
	CTime GetTime(LPCTSTR lpszTime);
	UINT GetInfo(CString cstrHeader, DWORD &dwContentLength, DWORD &dwStatusCode, CTime &TimeLastModified);
	BOOL StartTask(CString remoteurl, CString strDomain, CString localfile);
	ULONG GetDownloadBytes();
	CHTTPDownload();
	virtual ~CHTTPDownload();
public:
	CString GetLine(char* lpData, int& ndx);
	int GetHeadLength(char* lpData);
	UINT SendRequest(BOOL bHead = FALSE);
	BOOL ParseURL(CString str);
	//CTypedPtrList<CPtrList, CAsyncSocket*> m_lsTask;
	TCHAR m_szSavePath[512];
	TCHAR m_szTempSavePath[512];
	DOWNLOADSTATE m_state;
	BOOL m_bTerminate[4];

	//CString m_strHead;
	//CSocket m_pSocket;
	SOCKET m_socket;
	TCHAR m_szServer[256]; //IP 不带PORT
	TCHAR m_szDomain[256];//域名
	TCHAR m_szObject[4096];
	CString m_strReferer;
	CString m_strDownloadUrl;
	CTime   m_TimeLastModified;
	DWORD	m_dwDownloadSize;
	BOOL	m_bSupportResume;
	BOOL	m_bResume;
	DWORD	m_dwFileSize;
	UINT	m_nPort;

	CCriticalSection m_cs;
	LONG m_index;

	BOOL m_bSpeedTest;
private:
	ULONG m_nRecvBytes;//统计已经下载的字节数
	CritSec m_csSpeedTest;
};

#endif // !defined(AFX_HTTPDOWNLOAD_H__BA195238_6DC5_4491_9173_FEC73B994BDC__INCLUDED_)
