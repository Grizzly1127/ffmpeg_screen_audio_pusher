//Download by http://www.NewXing.com
// HTTPDownload.cpp: implementation of the CHTTPDownload class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CHTTPDownload.h"
#include "CharCode.h"
#include "LogEx.h"
#include "HttpGetPost.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTTPDownload::CHTTPDownload()
{
	for(int i = 0; i < 4; i++){
		m_bTerminate[i] = FALSE;
	}

	m_bSupportResume = FALSE;
	m_bResume = FALSE;
	memset(m_szSavePath, 0x00, sizeof(m_szSavePath));
	memset(m_szTempSavePath, 0x00, sizeof(m_szTempSavePath));
	memset(m_szServer, 0x00, sizeof(m_szServer));
	memset(m_szDomain, 0x00, sizeof(m_szDomain));
	memset(m_szObject, 0x00, sizeof(m_szObject));

	m_bSpeedTest = FALSE;
	m_nRecvBytes = 0;
}

CHTTPDownload::~CHTTPDownload()
{
	//m_lsTask.RemoveAll();
}

BOOL CHTTPDownload::StartTask(CString remoteurl, CString strDomain, CString localfile)
{
	if(!ParseURL(remoteurl))
	{
		remoteurl = _T("http://") + remoteurl;
		if(!ParseURL(remoteurl))
		{
			//TRACE("Requested URL is invalid!\n");
			Write_Log(LOG_ERROR, "HttpMutilDownload: Requested URL is invalid!.");
			return FALSE;
		}
	}

	wcscpy(m_szDomain, strDomain.GetBuffer(0));
	wcscpy(m_szSavePath, localfile.GetBuffer(0));
	//m_szSavePath = localfile.GetBuffer(0);
	wcscpy(m_szTempSavePath, m_szSavePath);
	wcscat(m_szTempSavePath, _T(".down"));
	//m_szTempSavePath = m_strSavePath + _T(".down");
	
	FILE* fp = NULL;
	CharCode charTemeSavePath(/*m_strTempSavePath.GetBuffer(0)*/m_szTempSavePath);
	if ((fp = fopen(charTemeSavePath.GetStringA(), "r")) == NULL)//检查是否有*.down文件
	{
		m_state.range[0] = -1;
		m_bResume = FALSE;
	}
	else//检查断点续传
	{
		m_bResume = TRUE;
		char* str = new char[1024];
		memset(str, 0, 1024);
		fgets(str, 1024, fp);
		m_state.url.Empty();
		m_state.url += str;
		m_state.url = m_state.url.Left(m_state.url.GetLength() - 2);
		memset(str, 0, 1024);
		fgets(str, 1024, fp);
		m_state.localfile.Empty();
		m_state.localfile += str;
		m_state.localfile = m_state.localfile.Left(m_state.localfile.GetLength() - 2);
		delete [] str;
		fread(&m_state.length, sizeof(LONG), 1, fp);
		fread(&m_state.time, sizeof(CTime), 1, fp);
		fread(m_state.range, sizeof(LONG), 8, fp);
		fclose(fp);
	}

	if(SendRequest() != SENDREQUEST_SUCCESS)
	{ 
		//TRACE("Remote Web Server is not reachable,or somthing else error occured!\n");
		Write_Log(LOG_ERROR, "HttpMutilDownload: Remote Web Server is not reachable,or somthing else error occured!.");
		return FALSE;
	}
	if(m_state.range[0] == -1)
	{
		m_state.localfile = localfile; 
		m_state.url = remoteurl;
		m_state.time = m_TimeLastModified;
		m_state.length = m_dwFileSize;
		for(int i = 0; i < 4; i++)
		{
			m_state.range[i * 2] = i * (m_dwFileSize / 4);
			m_state.range[i * 2 + 1] = (i + 1) * (m_dwFileSize / 4) - 1;
		}
		m_state.range[7] = m_dwFileSize - 1;
	}
	else
	{
		if(m_state.url != remoteurl)
		{
			AfxMessageBox(_T("Maybe the download file is not you want,you can try to save as another file!"));
			return FALSE;
		}
		if(m_state.time < m_TimeLastModified || m_state.range[7] != (LONG)(m_dwFileSize - 1))
		{
			m_state.time = m_TimeLastModified;
			m_state.length = m_dwFileSize;
			for(int i = 0; i < 4; i++)
			{
				m_state.range[i * 2] = i * (m_dwFileSize / 4);
				m_state.range[i * 2 + 1] = (i + 1) * (m_dwFileSize / 4);
			}
			m_state.range[7] = m_dwFileSize - 1;
		}
	}
	
	return TRUE;
}

BOOL CHTTPDownload::ParseURL(CString str)
{
	CString strURL = str;
	// 清除数据
	//m_strServer = _T("");
	//m_strObject = _T("");
	m_nPort	  = 0;

	int nPos = strURL.Find(_T("://"));
	if( nPos == -1 )
		return FALSE;

	// 进一步验证是否为http://
	CString strTemp = strURL.Left(nPos + lstrlen(_T("://")));
	strTemp.MakeLower();
	if (strTemp.Compare(_T("http://")) != 0)
		return FALSE;

	strURL = strURL.Mid( strTemp.GetLength() );
	nPos = strURL.Find('/');
	if ( nPos == -1 )
		return FALSE;

	wcscpy(m_szObject, strURL.Mid(nPos).GetBuffer(0));
	//m_strObject = strURL.Mid(nPos);
	strTemp   = strURL.Left(nPos);
	
	///////////////////////////////////////////////////////////////
	/// 注意：并没有考虑URL中有用户名和口令的情形和最后有#的情形
	/// 例如：http://abc@def:www.yahoo.com:81/index.html#link1
	/// 
	//////////////////////////////////////////////////////////////

	// 查找是否有端口号
	nPos = strTemp.Find(_T(":"));
	if( nPos == -1 )
	{
		wcscpy(m_szServer, strTemp.GetBuffer(0));
		//m_strServer = strTemp;
		m_nPort	  = DEFAULT_HTTP_PORT;
	}
	else
	{
		wcscpy(m_szServer, strTemp.Left(nPos).GetBuffer(0));
		//m_strServer = strTemp.Left( nPos );
		strTemp	  = strTemp.Mid( nPos+1 );
		m_nPort	  = (USHORT)_ttoi((LPCTSTR)strTemp);
	}
	return TRUE;
}

UINT CHTTPDownload::SendRequest(BOOL bHead)
{
	CString strVerb = _T("");
	CString strObject = m_szObject;
	if( bHead )
		strVerb = _T("HEAD ");
	else
		strVerb = _T("GET ");
	
	CString			strSend = _T("");
	CString			strHeader = _T("");
	CString			strRange = _T("");
	
	int				iStatus = 0,nRet;
	char			szReadBuf[4096];
	DWORD			dwContentLength,dwStatusCode;
	BOOL bResult = FALSE;
	
	while (TRUE)
	{
		//if(m_pSocket.m_hSocket != NULL)
		//	m_pSocket.Close();
		//m_pSocket.Create();
		//bResult = m_pSocket.Connect(m_strServer, m_nPort);
		//DWORD derr = GetLastError();

		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);
		SOCKADDR_IN addrServer;//服务端地址 
		m_socket = socket(AF_INET, SOCK_STREAM, 0);


		char szIP[256] = { 0 };
		struct hostent *pHostent = NULL;
		CharCode charServer(/*m_strServer.GetBuffer(0)*/m_szServer);

		//判断是否是域名
		if (NULL != _tcsstr(m_szServer, _T(".com")))
		{
			Write_LogW(LOG_INFO, _T("SendRequest: start gethostbyname."));
			if (NULL == (pHostent = gethostbyname(charServer.GetStringA()))) //使用gethostbyname前，确保已调用WSAStartup()
			{
				Write_Log(LOG_ERROR, "SendRequest: gethostbyname error(serv:%s, errno:%d).", charServer.GetStringA(), GetLastError());
				return SENDREQUEST_FAIL;
			}
			Write_LogW(LOG_INFO, _T("SendRequest: Gethostbyname success."));

			sockaddr_in   addr;
			addr.sin_addr.S_un.S_addr = *(u_long *)pHostent->h_addr_list[0];        // 转换到sockaddr结构中
			//if (NULL == inet_ntop(pHostent->h_addrtype, &addr.sin_addr, szIP, sizeof(szIP))) //xp不支持
			char *inetIP = NULL;
			if (NULL == (inetIP = inet_ntoa(addr.sin_addr)))
			{
				Write_LogW(LOG_ERROR, _T("SendRequest: get ip address error."));
				return SENDREQUEST_FAIL;
			}
			strncpy(szIP, inetIP, sizeof(szIP)-1);
		}
		else
		{
			strncpy(szIP, charServer.GetStringA(), sizeof(szIP)-1);
		}

		addrServer.sin_addr.S_un.S_addr = inet_addr(szIP);//目标IP
		addrServer.sin_family = AF_INET; addrServer.sin_port = htons(m_nPort);//连接端口
		Write_LogW(LOG_INFO, _T("SendRequest: start connect."));
		if(0 != connect(m_socket, (SOCKADDR*)&addrServer, sizeof(SOCKADDR)))
		{
			Write_Log(LOG_ERROR, "SendRequest: SendRequest connect server error(%s:%d,%d).", szIP, m_nPort,GetLastError());  // ***
			return SENDREQUEST_FAIL;
		}

		Write_Log(LOG_INFO, "SendRequest: Start construct protocol header.");
		strSend = strVerb + strObject + _T(" HTTP/1.1\r\n");
		strSend += _T("Host: ");
		//strSend += m_szServer; //需要传域名
		strSend += (m_szDomain[0]) ? m_szDomain : m_szServer;
		strSend += _T("\r\n");
		strSend += _T("User-Agent: eClass\r\n");
		strSend += _T("Accept: */*\r\n");
		strSend += _T("Pragma: no-cache\r\n");
		strSend += _T("Cache-Control: no-cache\r\n");
		if( !m_strReferer.IsEmpty() )
			strSend += _T("Referer: ") + m_strReferer + _T("\r\n");
		strSend += _T("Connection: close\r\n");
		strRange = _T("Range: bytes=100-\r\n");
		strSend += strRange;
		//必须要加一个空行，否则Http服务器将不会应答
		strSend += _T("\r\n");
		Write_Log(LOG_INFO, "SendRequest: Construct protocol header over.");
		//CharCode charSend(strSend.GetBuffer(0));
		string strAsend = CW2A(strSend.GetBuffer(0), CP_UTF8);
		//int ret = m_pSocket.Send(charSend.GetStringA(), strlen(charSend.GetStringA()));
		int ret = send(m_socket, strAsend.c_str(), strAsend.length(), 0);
		if (ret == SOCKET_ERROR)
		{
			Write_Log(LOG_ERROR, "CHTTPDownload::SendRequest error");
			closesocket(m_socket);
			WSACleanup();
			return SENDREQUEST_FAIL;
		}
		strSend.ReleaseBuffer();
		
		strHeader.Empty();
		SetNonBlocking(m_socket, true);
		int nTimeStart = 0;
		int nSleepMSec = 10;
		int nTimeout = 2000; //超时2秒
		while (nTimeStart < nTimeout / nSleepMSec)
		{
			nTimeStart++;
			ZeroMemory(szReadBuf, sizeof(szReadBuf));
			//ret = m_pSocket.Receive(szReadBuf, 1025);			
			ret = recv(m_socket, szReadBuf, sizeof(szReadBuf)-1, 0);
			if (ret <= SOCKET_ERROR)
			{
				//ERROR_INVALID_HANDLE这个错误有时会出现
				if (WSAEWOULDBLOCK == WSAGetLastError() || ERROR_INVALID_HANDLE == WSAGetLastError())
				{
					Sleep(nSleepMSec);
					continue;
				}
				else
				{
					Write_LogW(LOG_ERROR, _T(" CHTTPDownload::SendRequest recv error,ret:%d,WSAGetLastError:%0x"),ret, WSAGetLastError());
					break;
				}
			}
			else if (ret == 0)
			{
				Sleep(nSleepMSec);
				continue;
			}
			else if (ret>0)
			{
				if (szReadBuf[0] == '\0') // We have encountered "\r\n\r\n"
					break;

				CString strReadBuf = CA2W(szReadBuf, CP_UTF8);
				strHeader += strReadBuf;
				if (iStatus == 0)
					strHeader += _T("\r\n");

			}			
		}
		nRet = GetInfo(strHeader,dwContentLength,dwStatusCode,m_TimeLastModified);
		if (HTTP_OK != nRet)
		{
			CString cstrHeader = _T("");
			int nPos = strHeader.Find(_T("\r\n\r\n"));
			if (nPos != -1)
				cstrHeader = strHeader.Left(nPos);
			Write_LogW(LOG_ERROR, _T("SendRequest: SendRequest content error,nRet:%d"), nRet);  // adding
			Write_LogW(LOG_ERROR, _T("SendRequest: SendRequest content error,strSend:%s, strResult:%s."), strSend.GetBuffer(0), cstrHeader.GetBuffer(0));
		}
		Write_Log(LOG_INFO, "SendRequest: GetInfo result is %d", nRet);
		switch ( nRet )
		{
		case HTTP_FAIL:
			return SENDREQUEST_FAIL;
			break;
		case HTTP_ERROR:
			return SENDREQUEST_ERROR;
			break;
		case HTTP_REDIRECT:
			continue;
			break;
		case HTTP_OK:
			m_dwDownloadSize = dwContentLength + 100;
			// 应该判断一下服务器是否支持断点续传
			if( strRange.IsEmpty() )
				m_dwFileSize = dwContentLength + 100; // 整个文件的长度
			else
			{
				if ( dwStatusCode == 206 )	//支持断点续传
				{
					m_bSupportResume = TRUE;
					m_dwFileSize = dwContentLength + 100;
				}
				else						//不支持断点续传
				{
					m_bSupportResume = FALSE;
					m_dwFileSize = dwContentLength + 100;
				}
			}
			return SENDREQUEST_SUCCESS;
			break;
		default:
			return SENDREQUEST_FAIL;
			break;
		}
	}
	//m_pSocket.Close();
	closesocket(m_socket);
	return SENDREQUEST_SUCCESS;
}

//拆分HTTP响应头
UINT CHTTPDownload::GetInfo(CString cstrHeader, DWORD &dwContentLength,
							DWORD &dwStatusCode, CTime &TimeLastModified)
{
	Write_Log(LOG_INFO, "CHTTPDownload: GetInfo: Prase http heander.");
	dwContentLength = 0;
	dwStatusCode	= 0;
	TimeLastModified= CTime::GetCurrentTime();

	CString strHeader = cstrHeader;
	strHeader.MakeLower();

	//拆分出HTTP应答的头信息的第一行
	int nPos = strHeader.Find(_T("\r\n"));
	if (nPos == -1)
		return HTTP_FAIL;
	CString strFirstLine = strHeader.Left(nPos);

	// 获得返回码: Status Code
	strFirstLine.TrimLeft();
	strFirstLine.TrimRight();
	nPos = strFirstLine.Find(' ');
	if( nPos == -1 )
		return HTTP_FAIL;
	strFirstLine = strFirstLine.Mid(nPos+1);
	nPos = strFirstLine.Find(' ');
	if( nPos == -1 )
		return HTTP_FAIL;
	strFirstLine = strFirstLine.Left(nPos);
	dwStatusCode = (DWORD)_ttoi((LPCTSTR)strFirstLine);
	
	// 检查返回码
	if( dwStatusCode >= 300 && dwStatusCode < 400 ) //首先检测一下服务器的应答是否为重定向
	{
		nPos = strHeader.Find(_T("location:"));
		if (nPos == -1)
			return HTTP_FAIL;
		
		CString strRedirectFileName = strHeader.Mid(nPos + lstrlenW(_T("location:")));
		nPos = strRedirectFileName.Find(_T("\r\n"));
		if (nPos == -1)
			return HTTP_FAIL;

		strRedirectFileName = strRedirectFileName.Left(nPos);
		strRedirectFileName.TrimLeft();
		strRedirectFileName.TrimRight();
		
		// 设置Referer
		m_strReferer = m_strDownloadUrl;

		// 判断是否重定向到其他的服务器
		nPos = strRedirectFileName.Find(_T("http://"));
		if( nPos != -1 )
		{
			m_strDownloadUrl = strRedirectFileName;
			// 检验要下载的URL是否有效
			if ( !ParseURL(m_strDownloadUrl))
				return HTTP_FAIL;
			return HTTP_REDIRECT;
		}

		// 重定向到本服务器的其他地方
		strRedirectFileName.Replace(_T("\\"), _T("/"));
		
		// 是相对于根目录
		if( strRedirectFileName[0] == '/' )
		{
			wcscpy(m_szObject, strRedirectFileName.GetBuffer(0));
			//m_strObject = strRedirectFileName;
			return HTTP_REDIRECT;
		}
		
		// 是相对当前目录
		int nParentDirCount = 0;
		nPos = strRedirectFileName.Find(_T("../"));
		while (nPos != -1)
		{
			strRedirectFileName = strRedirectFileName.Mid(nPos+3);
			nParentDirCount++;
			nPos = strRedirectFileName.Find(_T("../"));
		}
		for (int i=0; i<=nParentDirCount; i++)
		{
			nPos = m_strDownloadUrl.ReverseFind('/');
			if (nPos != -1)
				m_strDownloadUrl = m_strDownloadUrl.Left(nPos);
		}
		m_strDownloadUrl = m_strDownloadUrl + _T("/") + strRedirectFileName;

		if ( !ParseURL(m_strDownloadUrl))
			return HTTP_FAIL;
		return HTTP_REDIRECT;
	}

	// 服务器错误，可以重试
	if( dwStatusCode >= 500 )
		return HTTP_ERROR;

	// 客户端错误，重试无用
	if( dwStatusCode >=400 && dwStatusCode <500 )
		return HTTP_FAIL;
		
	// 获取ContentLength
	nPos = strHeader.Find(_T("content-length:"));
	if (nPos == -1)
		return HTTP_FAIL;

	CString strDownFileLen = strHeader.Mid(nPos + lstrlenW(_T("content-length:")));
	nPos = strDownFileLen.Find(_T("\r\n"));
	if (nPos == -1)
		return HTTP_FAIL;

	strDownFileLen = strDownFileLen.Left(nPos);	
	strDownFileLen.TrimLeft();
	strDownFileLen.TrimRight();

	// Content-Length:
	dwContentLength = (DWORD) _ttoi( (LPCTSTR)strDownFileLen );

	// 获取Last-Modified:
	nPos = strHeader.Find(_T("last-modified:"));
	if (nPos != -1)
	{
		CString strTime = strHeader.Mid(nPos + lstrlenW(_T("last-modified:")));
		nPos = strTime.Find(_T("\r\n"));
		if (nPos != -1)
		{
			strTime = strTime.Left(nPos);
			strTime.TrimLeft();
			strTime.TrimRight();
			TimeLastModified = GetTime(strTime);
		}
	}
	return HTTP_OK;
}

CTime CHTTPDownload::GetTime(LPCTSTR lpszTime)
{
	int nDay,nMonth,nYear,nHour,nMinute,nSecond;

	CString strTime = lpszTime;
	int nPos = strTime.Find(',');
	if (nPos != -1)
	{
		strTime = strTime.Mid(nPos+1);
		strTime.TrimLeft();

		CString strDay,strMonth,strYear,strHour,strMinute,strSecond;
		CString strAllMonth = _T("jan,feb,mar,apr,may,jan,jul,aug,sep,oct,nov,dec");
		strDay = strTime.Left(2);
		nDay = _wtoi(strDay);
		strMonth = strTime.Mid(3,3);
		strMonth.MakeLower();
		nPos = strAllMonth.Find(strMonth);
		if (nPos != -1)
		{
			strMonth.Format(_T("%d"), ((nPos / 4) + 1));
			nMonth = _wtoi(strMonth);
		}
		else
			nMonth = 1;
		strTime = strTime.Mid(6);
		strTime.TrimLeft();
		nPos = strTime.FindOneOf(_T(" \t"));
		if (nPos != -1)
		{
			strYear = strTime.Left(nPos);
			nYear = _wtoi(strYear);
		}
		else
			nYear = 2000;
		strTime = strTime.Mid(nPos+1);
		strHour = strTime.Left(2);
		nHour = _wtoi(strHour);
		strMinute = strTime.Mid(3,2);
		nMinute = _wtoi(strMinute);
		strSecond = strTime.Mid(6,2);
		nSecond = _wtoi(strSecond);
	}
	
	CTime time(nYear,nMonth,nDay,nHour,nMinute,nSecond);
	return time;
}


UINT CHTTPDownload::ThreadFunc(int index)
{
	CString strObject = m_szObject;

	SOCKET hSocket = NULL;
	SOCKADDR_IN addrServer;//服务端地址 
	hSocket = socket(AF_INET, SOCK_STREAM, 0);


	char szIP[256] = { 0 };
	struct hostent *pHostent = NULL;
	CharCode charServer(/*m_strServer.GetBuffer(0)*/m_szServer);

	//判断是否是域名
	if (NULL != _tcsstr(m_szServer, _T(".com")))
	{
		if (NULL == (pHostent = gethostbyname(charServer.GetStringA())))
		{
			Write_Log(LOG_ERROR, "HttpMutilDownload: gethostbyname error(serv:%s).", charServer.GetStringA());
			return SENDREQUEST_FAIL;
		}

		sockaddr_in   addr;
		addr.sin_addr.S_un.S_addr = *(u_long *)pHostent->h_addr_list[0];        // 转换到sockaddr结构中
		//if (NULL == inet_ntop(pHostent->h_addrtype, &addr.sin_addr, szIP, sizeof(szIP)))
		char *inetIP = NULL;
		if (NULL == (inetIP = inet_ntoa(addr.sin_addr)))
		{
			
			Write_LogW(LOG_ERROR, _T("HttpMutilDownload: get ip address error."));
			return SENDREQUEST_FAIL;
		}
		strncpy(szIP, inetIP, sizeof(szIP)-1);

	}
	else
	{
		strncpy(szIP, charServer.GetStringA(), sizeof(szIP) - 1);
	}

	addrServer.sin_addr.S_un.S_addr = inet_addr(szIP);//目标IP 
	addrServer.sin_family = AF_INET; 
	addrServer.sin_port = htons(m_nPort);//连接端口
	Write_LogW(LOG_INFO, _T("HttpMutilDownload: start connect."));
	int nconnRet = connect(hSocket, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
	if (0 != nconnRet)
	{
		Write_Log(LOG_ERROR, "HttpMutilDownload: connect serv error(serv:%s,ret:%d,errno:%d).", szIP, nconnRet,GetLastError()); // ***
		return SENDREQUEST_FAIL;
	}

	//pSocket.Create();
	//pSocket.Connect(m_strServer, m_nPort);
	
	CString strSend, strRange, strHeader;
	char szReadBuf[1025];
	strSend = _T("GET ") + strObject + _T(" HTTP/1.1\r\n");
	strSend += _T("Host: ");
	//strSend += m_szServer;//需要传域名
	strSend += (m_szDomain[0]) ? m_szDomain : m_szServer;
	strSend += _T("\r\n");
	strSend += _T("Accept: */*\r\n");
	strSend += _T("Pragma: no-cache\r\n");
	strSend += _T("Cache-Control: no-cache\r\n");
	if( !m_strReferer.IsEmpty() )
		strSend += _T("Referer: ") + m_strReferer + _T("\r\n");
	strSend += _T("Connection: close\r\n");
	strRange.Format(_T("Range: bytes=%d-%d\r\n"), m_state.range[2 * index], m_state.range[2 * index + 1]);
	if(m_bSupportResume)
		strSend += strRange;
	//必须要加一个空行，否则Http服务器将不会应答
	strSend += _T("\r\n");
	CharCode charSend(strSend.GetBuffer(0));
	//int ret = pSocket.Send(charSend.GetStringA(), strlen(charSend.GetStringA()));
	int ret = send(hSocket, charSend.GetStringA(), strlen(charSend.GetStringA()), 0);
	if (ret < 0)
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: send data error(ret:%d)."), ret);
		return SENDREQUEST_FAIL;
	}

	strSend.ReleaseBuffer();

	strHeader.Empty();
	int iStatus = 0;
	ZeroMemory(szReadBuf,1025);
	//ret = pSocket.Receive(szReadBuf, 1025);
	ret =  recv(hSocket, szReadBuf, 1025, 0);
	if (ret < 0)
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: recv data error(ret:%d)."), ret);
		return SENDREQUEST_FAIL;
	}

	//处理404错误
	CStringA strNotFound = "HTTP/1.1 404 Not Found";
	CStringA strRecvDate = szReadBuf;
	if (-1 != strRecvDate.Find(strNotFound))
	{
		Write_LogW(LOG_ERROR, _T("CHTTPDownload: recv 404 not find, %s."), strObject.GetBuffer(0));
		return SENDREQUEST_FAIL;
	}

	int n = GetHeadLength(szReadBuf);		
	Write_Log(LOG_INFO, "HttpMutilDownload: GetHeadLength result is %d.", n);
	CFile hFile;
	CString strFileName;
	strFileName.Format(_T("%s%d"), /*m_strTempSavePath*/m_szTempSavePath, index);

	BOOL bOpen = FALSE;
	CFileException e;
	if(!m_bResume)
		bOpen = hFile.Open(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::modeRead,&e);  // adding 'CFile::modeRead' by 0912
	else
		bOpen = hFile.Open(strFileName, CFile::modeWrite | CFile::modeRead,&e);  // adding 'CFile::modeRead' by 0912

	if (bOpen == FALSE)
	{
		//TRACE("Error in file open!\n");
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: Error in file open, file name = %s, errorcode=%d!"), strFileName.GetBuffer(0), e.m_cause);
		return -10;
	}
	hFile.SeekToEnd();
	//TRACE("Entering the key section!\n");
	if (ret - n < 0)
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: recv data error(%d,%d)."), ret, n);
		hFile.Close();
		return SENDREQUEST_FAIL;
	}
	
	hFile.Write(szReadBuf + n, ret - n); //socket send 或者recv不成功的情况下，此处会崩溃，前面加个判断
	int sum = ret - n, num = 0;
	SetNonBlocking(hSocket, true);//设置非阻塞,阻塞的recv线程不会退出，file不能被close
	int nSleepMSec = 10;
	int nTimeout = 0;
	if (-1 != strObject.Find(_T(".jpg")))
	{
		nTimeout = 4000; //图片 注意：此线程超时时间应 <= 主线程CHttpDownloadEx超时时间, 否则主线程退出，此线程中打开的CFile没有正常cloae
	}
	else
	{
		nTimeout = 70000; //音频
	}
	int nTimeStart = 0;
	for (nTimeStart = 0; nTimeStart < nTimeout / nSleepMSec; nTimeStart++)
	{
		if (index > 4)
		{
			hFile.Close();
			return -10;
		}

		if(m_bTerminate[index])
		{
			m_state.range[2 * index] = m_state.range[2 * index] + sum;
			hFile.Close();
			return -5;
		}
		ZeroMemory(szReadBuf,1025);
		//if(!(num = pSocket.Receive(szReadBuf, 1025)) || num == SOCKET_ERROR)
		num = recv(hSocket, szReadBuf, 1025, 0);
		if (num <= SOCKET_ERROR)
		{
			//ERROR_INVALID_HANDLE这个错误有时会出现
			if (WSAEWOULDBLOCK == WSAGetLastError() || ERROR_INVALID_HANDLE == WSAGetLastError())
			{
				Sleep(nSleepMSec);
				continue;
			}
			else
			{
				Write_LogW(LOG_ERROR, _T("HttpMutilDownload: recv file data error(%d)."), num);
				hFile.Close();
				return -6;
			}
		}
		else if (num == 0)
		{
			break;
		}
		else if (0 < num)
		{
			hFile.Write(szReadBuf, num);
			sum += num;

			if (m_bSpeedTest)
			{
				AutoLock l(&m_csSpeedTest);
				m_nRecvBytes += num;
			}
		}
	}

	if (nTimeout / nSleepMSec <= nTimeStart)
	{
		Write_LogW(LOG_ERROR, _T("HttpMutilDownload: recv file data timeout, filename:%s"), strFileName.GetBuffer(0));
		hFile.Close();
		return -7;
	}
	
	hFile.Close();
	return 0;
}

int CHTTPDownload::GetHeadLength(char *lpData)
{
	int ndx = 0;
	CString str;
	while(1)
	{
		str.Empty();
		str = GetLine(lpData, ndx);
		if(str.IsEmpty())
			break;
	}
	return (ndx);
}

//通过获取http头（1025个字节）中最后一个0a
CString CHTTPDownload::GetLine(char *lpData, int& ndx)
{
	BOOL bLine = FALSE;
	CString str;
	while ( bLine == FALSE && ndx < 1025 )
	{
		char ch = (char)(lpData[ndx]);
		switch( ch )
		{
		case '\r': // ignore
			break;
		case '\n': // end-of-line
			bLine = TRUE;
			break;
		default:   // other....
			str += ch;
			break;
		}
		++ndx;
	}
	return str;
}

ULONG CHTTPDownload::GetDownloadBytes()
{
	AutoLock l(&m_csSpeedTest);
	return m_nRecvBytes;
}