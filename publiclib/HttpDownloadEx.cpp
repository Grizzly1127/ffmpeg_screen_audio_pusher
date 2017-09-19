#include "stdafx.h"
#include "HttpDownloadEx.h"
#include "LogEx.h"
#include "CharCode.h"


#include <wininet.h>
#pragma comment(lib,"Wininet.lib")

CHttpDownloadEx::CHttpDownloadEx()
{
	m_bResult = TRUE;
	m_bSpeedTest = FALSE;
}


CHttpDownloadEx::~CHttpDownloadEx()
{
	StopDownThread();
	WaitForStop();
}

void CHttpDownloadEx::ThreadProcMain(void)
{
	Write_LogW(LOG_INFO, _T("HttpMutilDownload: ThreadProcMain, %s,  domain:%s ,saveFile:%s"), m_strUrl.GetBuffer(0), m_strDomain.GetBuffer(0), m_strSaveFile.GetBuffer(0));
    m_bResult = TRUE;
	if (!m_http.StartTask(m_strUrl.GetBuffer(0),m_strDomain.GetBuffer(0), m_strSaveFile.GetBuffer(0)))
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: starttask error.%s, %s"), m_strUrl.GetBuffer(0), m_strSaveFile.GetBuffer(0));
		m_bResult = FALSE;
		return;
	}

	//超时时间
	DWORD nTimeout = 0;
	if (-1 != m_strUrl.Find(_T(".jpg")))
	{
		nTimeout = 5000; //图片
	}
	else
	{
		nTimeout = 72000; //音频
	}

	//create 4 threads
	HANDLE m_hThread[4];
	if (m_http.m_bSupportResume)
	{
		//TRACE("create four thread to download!\n");
		Write_LogW(LOG_INFO, _T("HttpMutilDownload: create four thread to download!, %s,  domain:%s"), m_strUrl.GetBuffer(0), m_strDomain.GetBuffer(0));
		DWORD dwThread[4];
		m_http.m_index = 0;
		for (int i = 0; i < 4; i++)
		{
			m_http.m_bTerminate[i] = FALSE;
			m_hThread[i] = ::CreateThread(NULL, 0, DownloadThreadEx, (LPVOID)&m_http, 0, &dwThread[i]);
			if (m_hThread[i] == NULL)
			{
				//TRACE("Create Thread Error!\n");
				Write_Log(LOG_ERROR, "HttpMutilDownload: CreateThread fail.");
				m_bResult = FALSE;
				return;
			}
		}
		Write_Log(LOG_INFO, "HttpMutilDownload: [%08x] [%08x] [%08x] [%08x]", dwThread[0], dwThread[1], dwThread[2], dwThread[3]);
	}
	else
	{
		//TRACE("We will create only one thread to download!\n");
		Write_Log(LOG_INFO, "HttpMutilDownload: create only one thread to download!");
		DWORD dwThread;
		m_http.m_index = 0;
		m_http.m_bTerminate[0] = FALSE;
		m_hThread[0] = ::CreateThread(NULL, 0, DownloadThreadEx, (LPVOID)&m_http, 0, &dwThread);
		if (m_hThread[0] == NULL)
		{
			//TRACE("Create Thread Error!\n");
			Write_Log(LOG_INFO, "HttpMutilDownload: Create Thread Error!");
			m_bResult = FALSE;
			return;
		}
	}

	//whait for finish
	char* lpData = NULL;
	Write_Log(LOG_INFO, "CHttpDownload: m_http.m_bSupportResume:%s!", m_http.m_bSupportResume ? "TRUE": "FALSE");
	if (m_http.m_bSupportResume)
	{
		//等待线程退出
		HRESULT ret = WaitForMultipleObjects(4, m_hThread, TRUE, nTimeout); //超时12秒，改为18秒（xk）
		Write_Log(LOG_INFO, "CHttpDownload : WaitForMultipleObjects ret:%d!", ret); //WAIT_TIMEOUT :258
		//如果被强行结束线程
		if (m_http.m_bTerminate[0])
		{
			m_bResult = FALSE;
			return;
		}
		if (ret != 0)
		{
			m_bResult = FALSE;
			Write_LogW(LOG_ERROR, _T("CHttpDownload : Download file error:(%s, %s) WaitForMultipleObjects ret:%d!"), 
				m_strUrl.GetBuffer(0), m_strSaveFile.GetBuffer(0), ret);
			return;
		}
		else if (ret == 0)
		{
			CFile hFileWrite, hFileRead[4];
			CString strFileTempName;
			Write_LogW(LOG_INFO, _T("CHttpDownload : Began to merge files : %s!"), m_strSaveFile.GetBuffer(0));
			if (hFileWrite.Open(m_strSaveFile.GetBuffer(0), CFile::modeCreate | CFile::modeWrite))
			{
				for (int i = 0; i < 4; i++)
				{
					strFileTempName.Empty();
					strFileTempName.Format(_T("%s.down%d"), m_strSaveFile.GetBuffer(0), i);

					if (hFileRead[i].Open(strFileTempName, CFile::modeRead))
					{
						DWORD len = (DWORD)hFileRead[i].GetLength();
						if ((len == 0) || (len>(50*1024*1024)))
						{
							Write_LogW(LOG_ERROR, _T("CHttpDownload : read files len error!(%s)"),
								strFileTempName.GetBuffer(0));
							hFileWrite.Close();
							hFileRead[i].Close();
							m_bResult = FALSE;
							return;
						}
						
						lpData = new char[len];
						if (lpData)
						{
							hFileRead[i].Read(lpData, len);
							hFileWrite.Write(lpData, len);
							hFileRead[i].Close();
							delete[] lpData;
							lpData = NULL;
						}
						DeleteFile(strFileTempName);
					}
					else
					{
						Write_LogW(LOG_ERROR, _T("CHttpDownload: open file fail.(%s)"), strFileTempName.GetBuffer(0));
						hFileWrite.Close();
						m_bResult = FALSE;
						return;
					}
				}
				hFileWrite.Close();
			}
			else
			{
				Write_LogW(LOG_ERROR, _T("CHttpDownload: Create file fail.(%s)"), m_strSaveFile.GetBuffer(0));
				m_bResult = FALSE;
			}
			if (m_bResult)
				Write_Log(LOG_INFO, "CHttpDownload : Merge files success!");
			else
				Write_Log(LOG_ERROR, "CHttpDownload : Merge files fail!");
			return;
		}//end if (ret == 0)
	}//end if (m_http.m_bSupportResume)
	else
	{
		//HRESULT ret = WaitForMultipleObjects(1, m_hThread, TRUE, INFINITE);
		HRESULT ret = WaitForMultipleObjects(4, m_hThread, TRUE, nTimeout); //超时12秒，改为18秒（xk）
		if (m_http.m_bTerminate[0])
		{
			m_bResult = FALSE;
			return;
		}
		if (ret != 0)
		{
			m_bResult = FALSE;
			Write_LogW(LOG_ERROR, _T("CHttpDownload : Download file error:(%s, %s) WaitForMultipleObjects ret:%d!"),
				m_strUrl.GetBuffer(0), m_strSaveFile.GetBuffer(0), ret);
			return;
		}
		else if (ret == 0)
		{
			CStringA name;
			CharCode charLocal(m_strSaveFile.GetBuffer(0));

			name = charLocal.GetStringA(); name += (".down0");
			rename(name.GetBuffer(0), charLocal.GetStringA());
		}
		m_bResult = TRUE;
		return;
	}
	m_bResult = TRUE;
	return;
}

bool CHttpDownloadEx::HttpSingleDownload(CString strFileNameAll, CString strUrl)
{
	return (HttpSingleDownloadByInternet(strFileNameAll, strUrl) || HttpSingleDownloadBySock(strFileNameAll, strUrl));
}

bool CHttpDownloadEx::HttpSingleDownloadBySock(CString strFileNameAll, CString strUrl)
{
	if (_T("") == strFileNameAll || _T("") == strUrl)
	{
		Write_LogW(LOG_ERROR, _T("CHttpDownloadEx: HttpSingleDownload paramer error"));
		return false;
	}

	CHTTPDownload http;
	http.m_bSupportResume = FALSE;

	if (!http.StartTask(strUrl, _T(""), strFileNameAll))
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: starttask error.%s, %s"), strUrl.GetBuffer(0), strFileNameAll.GetBuffer(0));
		return false;
	}

	Write_Log(LOG_INFO, "HttpMutilDownload: create only one thread to download!");
	DWORD dwThread;
	http.m_index = 0;
	http.m_bTerminate[0] = FALSE;
	HANDLE m_hThread[4];
	m_hThread[0] = ::CreateThread(NULL, 0, DownloadThreadEx, (LPVOID)&http, 0, &dwThread);
	if (m_hThread[0] == NULL)
	{
		return false;
	}

	HRESULT ret = WaitForMultipleObjects(1, m_hThread, TRUE, 60000);      // 将5000，改为60000（xk）
	if (http.m_bTerminate[0])
	{
		return false;
	}

	if (ret == 0)
	{
		CStringA name;
		CharCode charLocal(strFileNameAll.GetBuffer(0));
		name = charLocal.GetStringA(); name += (".down0");
		DeleteFile(strFileNameAll.GetBuffer(0));
		if (rename(name.GetBuffer(0), charLocal.GetStringA()) == 0)
			return true;
		else
			return false;
	}
	else
	{
		Write_LogW(LOG_ERROR, _T("HttpSingleDownload timeout, WaitForMultipleObjects ret.%d"), GetLastError());
		return false;
	}

	return true;
}

bool CHttpDownloadEx::HttpSingleDownloadByInternet(CString strFileNameAll, CString strUrl)
{
	//检查是否为空
	if (strFileNameAll.IsEmpty() || strUrl.IsEmpty())
		return false;

	HINTERNET hInternet = NULL, hInternetFile = NULL;
	hInternet = InternetOpen(_T("Download From HTTP"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if (hInternet == NULL)
	{
		Write_LogW(LOG_ERROR, _T("CHttpDownloadEx: HttpSingleDownload InternetOpen error,strUrl:%s."), strUrl.GetBuffer(0));
		return false;
	}

	TCHAR szHead[] = _T("Accept: */*\r\n\r\n");
	int nHeadLen = _tcslen(szHead);
	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start InternetOpenUrl"));
	hInternetFile = InternetOpenUrl(hInternet, strUrl, szHead, nHeadLen, INTERNET_FLAG_RELOAD, 0);
	if (hInternetFile == NULL)
	{
		Write_LogW(LOG_ERROR, _T("CHttpDownloadEx: HttpSingleDownload InternetOpenUrl error,strUrl:%s."), strUrl.GetBuffer(0));
		return false;
	}
	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start InternetOpenUrl success"));

	DWORD dwByteToRead = 0;
	DWORD dwSizeofRequest = 4;
	//DWORD dwReadBytes = 0;

	int nPercent = 0;
	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start HttpQueryInfo"));
	BOOL bQuery = HttpQueryInfo(hInternetFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwByteToRead, &dwSizeofRequest, NULL);
	if (!bQuery)
	{
		dwByteToRead = 0;
		Write_LogW(LOG_ERROR, _T("CHttpDownloadEx: HttpSingleDownload HttpQueryInfo error,strUrl:%s."), strUrl.GetBuffer(0));
		return false;
	}
	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start HttpQueryInfo success"));

	DWORD dwBytes = 0;
	BYTE buffer[1024];
	FILE *fp = _tfopen(strFileNameAll, _T("wb+"));
	if (fp == NULL)
	{
		//AfxMessageBox(_T("创建本地文件失败!"));
		Write_Log(LOG_ERROR, "CHttpDownloadEx: HttpSingleDownload _tfopen error(errCode:%d).", GetLastError());
		return false;
	}

	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start InternetReadFile"));
	while (true)
	{
		if (!InternetReadFile(hInternetFile, buffer, 1024, &dwBytes))
		{
			Write_LogW(LOG_ERROR, _T("CHttpDownloadEx: HttpSingleDownload InternetReadFile error,strUrl:%s."), strUrl.GetBuffer(0));
			return false;
		}
		if (dwBytes == 0)
			break; //当没有文件可读了，便退出while
		fwrite(buffer, sizeof(BYTE), dwBytes, fp);	//写文件	
	}
	Write_LogW(LOG_INFO, _T("CHttpDownloadEx: Start InternetReadFile success"));
	fclose(fp);

	return true;
}

bool CHttpDownloadEx::HttpMutilDownload(TCHAR* szUrl, TCHAR* szDomain, TCHAR *szSaveFile)
{
	m_strUrl = szUrl;
	m_strDomain = szDomain;
	m_strSaveFile = szSaveFile;
	m_http.m_bSpeedTest = m_bSpeedTest;
	StartThread();
	WaitForStop();
	return (bool)m_bResult;
}

void CHttpDownloadEx::StopDownThread()
{
	for (int i = 0; i <= 4; ++i)
	{
		m_http.m_bTerminate[i] = TRUE;
	}
}


DWORD WINAPI DownloadThreadEx(LPVOID lpParam)
{

	CHTTPDownload* pThis = (CHTTPDownload*)lpParam;

	int index;
	index = pThis->m_index;
	InterlockedIncrement(&pThis->m_index);
	if (!pThis)
	{
		Write_Log(LOG_ERROR, "DownloadThread's lpParam is null.");
	}

	int ret = pThis->ThreadFunc(index);
	if (ret)
	{
		Write_Log(LOG_ERROR, "DownloadThread's ThreadFunc error ret:%d.", ret);
	}
	//TRACE("Thread %d has successfully finsihed!%d\n", index, ret);
	return 0L;
}

ULONG CHttpDownloadEx::GetDownloadBytes()
{
	return m_http.GetDownloadBytes();
}