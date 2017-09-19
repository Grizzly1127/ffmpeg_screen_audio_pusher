#include "stdafx.h"
#include "HttpGetPost.h"
#include "wininet.h"  
#include "afxinet.h"
#include "LogEx.h"
#include "NetAdapter.h"

#include <errno.h>    // adding by 0912

bool SetNonBlocking(SOCKET hSocket, bool bNB)
{
#ifdef WIN32
	unsigned long lValue = bNB ? 1 : 0;
	//ioctlsocket控制套接口模式
	//FIONBIO允许或禁止套接口的非阻塞模式，非阻塞模式lValue为非零，阻塞模式lValue为零。创建一个套接口时，默认处于阻塞模式。
	int nRet = ioctlsocket(hSocket, FIONBIO, &lValue);
	return (nRet == 0);
#else
	if (bNB)
	{
		if (fcntl(hSocket, F_SETFL, O_NONBLOCK) == -1)
		{
			return false;
		}
	}
	else
	{
		if (fcntl(hSocket, F_SETFL, 0) == -1)
		{
			return false;
		}
	}
	return true;
#endif
}

CHttpGetPost::CHttpGetPost()
: m_strTable(_T(""))
{
}


CHttpGetPost::~CHttpGetPost()
{
}

// 添加字段
void CHttpGetPost::AddParam(CStringA strKey, CStringA strValue)
{
	if (!m_strTable.IsEmpty())
		m_strTable += "&";

	strValue.Replace("/", "%2f");
	strValue.Replace("+", "%2b");
	strValue.Replace(" ", "%20");
	m_strTable = m_strTable + strKey + "=" + strValue;

}

void CHttpGetPost::AddParam(CStringA strKey, int nValue)
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	CharCode charValue(strValue.GetBuffer(0));
	AddParam(strKey, charValue.GetStringA());
}

// 以http Get方式请求URL
CString   CHttpGetPost::doGet(CString   href)
{
	CString   httpsource = _T("");
	CInternetSession   session1(NULL, 0);
	CHttpFile*   pHTTPFile = NULL;
	try{
		pHTTPFile = (CHttpFile*)session1.OpenURL(href);
		//session1.  
	}
	catch (CInternetException)
	{
		pHTTPFile = NULL;
	}
	if (pHTTPFile)
	{
		CString   text;
		for (int i = 0; pHTTPFile->ReadString(text); i++)
		{
			httpsource = httpsource + text + _T("\r\n");
		}
		pHTTPFile->Close();
		delete   pHTTPFile;
	}
	else
	{

	}
	return   httpsource;
}
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename)
{
	string::size_type n;
	string url = mUrl;

	if (url.substr(0, 7) == "http://")
		url.erase(0, 7);

	if (url.substr(0, 8) == "https://")
		url.erase(0, 8);

	n = url.find('/');
	if (n != string::npos)
	{
		serverName = url.substr(0, n);
		filepath = url.substr(n);
		n = filepath.rfind('/');
		filename = filepath.substr(n + 1);
	}

	else
	{
		serverName = url;
		filepath = "/";
		filename = "";
	}
}

SOCKET connectToServer(char *szServerName, WORD portNum)
{
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	SOCKET conn;

	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return NULL;

	if (inet_addr(szServerName) == INADDR_NONE)
	{
		hp = gethostbyname(szServerName);
	}
	else
	{
		addr = inet_addr(szServerName);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (hp == NULL)
	{
		closesocket(conn);
		return NULL;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(portNum);
	if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
	{
		closesocket(conn);
		return NULL;
	}
	return conn;
}

int getHeaderLength(char *content)
{
	const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
	char *findPos;
	int ofset = -1;

	findPos = strstr(content, srchStr1);
	if (findPos != NULL)
	{
		ofset = findPos - content;
		ofset += strlen(srchStr1);
	}

	else
	{
		findPos = strstr(content, srchStr2);
		if (findPos != NULL)
		{
			ofset = findPos - content;
			ofset += strlen(srchStr2);
		}
	}
	return ofset;
}

char * CHttpGetPost::doGet(char*href)
{
	const int bufSize = 1024 * 10;
	char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;
	mParseUrl(href, server, filepath, filename);
	conn = connectToServer((char*)server.c_str(), 80);
	sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
	strncpy(sendBuffer, tmpBuffer, sizeof(sendBuffer)-1);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Host: %s", server.c_str());
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	strcat(sendBuffer, "\r\n");
	send(conn, sendBuffer, strlen(sendBuffer), 0);
	printf("Buffer being sent:\n%s", sendBuffer);
	totalBytesRead = 0;

	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);
		if (thisReadSize <= 0)
			break;
		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);
		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}
	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	delete(tmpResult);
	closesocket(conn);
	return(result);
}
// 以Http Post方式请求URL
CString CHttpGetPost::doPost(CString   href)
{
	CString strRec = _T("");
	CInternetSession hSession;
	CHttpConnection* hConnet = NULL;
	CHttpFile* pHttpFile = NULL;
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	AfxParseURL((LPCTSTR)href, dwServiceType, strServerName, strObject, nPort);
	DWORD dwCode = 0;
	try
	{
		hSession.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000);//5秒的发送数据超时
		hSession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);//5秒的接收数据超时
		hConnet = hSession.GetHttpConnection(strServerName, nPort);
		if (hConnet)
		{
			pHttpFile = hConnet->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, _T("HTTP/1.1"), INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT);
			
			if (pHttpFile)
			{
				pHttpFile->AddRequestHeaders(_T("Content-Type:   application/x-www-form-urlencoded"));
				pHttpFile->AddRequestHeaders(_T("Accept:   */*"));
				pHttpFile->SendRequest(NULL, 0, m_strTable.GetBuffer(0), m_strTable.GetLength());
				pHttpFile->QueryInfoStatusCode(dwCode);
			}
		}
	}
	catch (CInternetException   *   e)
	{
		TCHAR szErr[1024];
		e->GetErrorMessage(szErr, 1024);
		TRACE(szErr);
		e->Delete();
	};

	if (pHttpFile)
	{
		CString   strText;
		for (int i = 0; pHttpFile->ReadString(strText); i++)
		{
			strRec = strRec + strText + _T("\r\n");
		}

		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = NULL;

		delete hConnet;
		hConnet = NULL;

		hSession.Close();
	}
	else
	{
		// do anything.....
	}

	return   strRec;
}


CString CHttpGetPost::doPost(CString href, CString strFilePath, CString strFileName, CString strVersion, CString strGroupID, CString strUserID, CString strRoomID, CString strVerificationKey)
{
	CString strRec = _T("");
	CFile fTrack;
	CInternetSession hSession;
	CHttpConnection* hConnet = NULL;
	CHttpFile* pHttpFile = NULL;
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	CString strHTTPBoundary = _T("----WebKitFormBoundaryCJsAP52jsafp27FY");//定义边界值;
	CString strPreFileData;
	CString strEndFileData;
	DWORD dwTotalRequestLength;

	//解析URL
	AfxParseURL((LPCTSTR)href, dwServiceType, strServerName, strObject, nPort);
	if (FALSE == fTrack.Open(strFilePath, CFile::modeRead | CFile::shareDenyWrite))//读出文件 
	{
		return _T("");
	}

	strPreFileData = PreFileData(strHTTPBoundary, strFileName, strVersion, strGroupID, strUserID, strRoomID, strVerificationKey);
	strEndFileData = EndFileData(strHTTPBoundary);
	dwTotalRequestLength = strPreFileData.GetLength() + strEndFileData.GetLength() + (DWORD)fTrack.GetLength();

	try
	{
		hConnet = hSession.GetHttpConnection(strServerName.GetBuffer(0), nPort);
		if (hConnet)
		{
			pHttpFile = hConnet->OpenRequest(CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, _T("HTTP/1.1"), INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT);
			if (pHttpFile)
			{
				pHttpFile->AddRequestHeaders(MakeRequestHeaders(strHTTPBoundary));//发送包头请求
				CString strFormat = _T("");
				strFormat.Format(_T("Content-Type: multipart/form-data; boundary=%s\r\n"), strHTTPBoundary);
				pHttpFile->AddRequestHeaders(strFormat);
				pHttpFile->SendRequestEx(dwTotalRequestLength, HSR_SYNC | HSR_INITIATE);


#ifdef _UNICODE
				USES_CONVERSION;
				pHttpFile->Write(W2A(strPreFileData), strPreFileData.GetLength());
#else
				pHttpFile->Write((LPSTR)(LPCSTR)strPreFileData, strPreFileData.GetLength());//写入服务器所需信息
#endif
				BYTE pBuffer[1024];
				DWORD readed;
				while ((readed = fTrack.Read(pBuffer, 1024)) > 0)
				{
					pHttpFile->Write(pBuffer, readed);
				}

#ifdef _UNICODE
				pHttpFile->Write(W2A(strEndFileData), strEndFileData.GetLength());
#else
				pHttpFile->Write((LPSTR)(LPCSTR)strEndFileData, strEndFileData.GetLength());//写入服务器所需信息
#endif
				pHttpFile->EndRequest(HSR_SYNC);
			}
		}
	}
	//catch (CException* e)
	catch (...)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost: Post exception, URL = %s"), href.GetBuffer(0));
		//删除http文件对象
		if (pHttpFile != NULL)
		{
			delete pHttpFile;
			pHttpFile = NULL;
		}
		//删除http连接对象
		if (hConnet != NULL)
		{
			delete hConnet;
			hConnet = NULL;
		}
		//关闭http连接
		hSession.Close();
	}

	if (pHttpFile)
	{
		CString   strText;
		for (int i = 0; pHttpFile->ReadString(strText); i++)
		{
			strRec = strRec + strText + _T("\r\n");
		}

		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = NULL;

		delete hConnet;
		hConnet = NULL;

		hSession.Close();

	}
	else
	{
		// do anything.....
	}

	fTrack.Close();
	Write_LogW(LOG_INFO, _T("CHttpGetPost: Post over, URL = %s"), href.GetBuffer(0));
	return   strRec;
}

CString CHttpGetPost::PreFileData(CString strBoundary, CString strFileName, CString strVersion, CString strGroupID, CString strUserID, CString strRoomID, CString strVerificationKey)
{
	CString strFormat;
	CString strData;

	//以下字段需要按照固定格式填写
	//Source
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"Source\"");//传给网络上的参数，根据网站抓包查看到底是需要哪些
	strFormat += _T("\r\n\r\n");
	strFormat += _T("windows_log");
	strFormat += _T("\r\n");

	//UA
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"UA\"");//传给网络上的参数，根据网站抓包查看到底是需要哪些
	strFormat += _T("\r\n\r\n");
	strFormat += _T("Windows");
	strFormat += _T("\r\n");

	//Version
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"Version\"");//传给网络上的参数，根据网站抓包查看到底是需要哪些
	strFormat += _T("\r\n\r\n");
	strFormat += strVersion;
	strFormat += _T("\r\n");

	//GroupID
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"GroupID\"");
	strFormat += _T("\r\n\r\n");
	strFormat += strGroupID;
	strFormat += _T("\r\n");

	//UserID
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"UserID\"");
	strFormat += _T("\r\n\r\n");
	strFormat += strUserID;
	strFormat += _T("\r\n");

	//RoomID
	if (_T("") != strRoomID)
	{
		strFormat += _T("--");
		strFormat += strBoundary;
		strFormat += _T("\r\n");
		strFormat += _T("Content-Disposition: form-data; name=\"RoomID\"");
		strFormat += _T("\r\n\r\n");
		strFormat += strRoomID;
		strFormat += _T("\r\n");
	}

	//VerificationKey
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"VerificationKey\"");
	strFormat += _T("\r\n\r\n");
	strFormat += strVerificationKey;
	strFormat += _T("\r\n");

	//FileName
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"FileName\"");
	strFormat += _T("\r\n\r\n");
	strFormat += strFileName;
	strFormat += _T("\r\n");

	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"file\"; filename=\"");//文件地址信息
	strFormat += strFileName;
	strFormat += _T("\"");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Type: application/octet-stream");

	strFormat += _T("\r\n\r\n");
	strData = strFormat;
	return strData;
}

CString CHttpGetPost::EndFileData(CString strBoundary)
{
	CString strFormat;
	CString strData;

	strFormat += _T("\r\n");
	strFormat += _T("--");
	strFormat += strBoundary;
	strFormat += _T("--");
	strFormat += _T("\r\n");
	
	strData = strFormat;
	return strData;
}

CString CHttpGetPost::MakeRequestHeaders(CString &strBoundary)
{
	CString strFormat;
	CString strData;

	strFormat = _T("Content-Type: multipart/form-data; boundary=%s\r\n");//二进制文件传送Content-Type类型为: multipart/form-data

	strData.Format(strFormat, strBoundary);
	return strData;
}

CStringA CHttpGetPost::doPost2(CString href, CString slaveHref, CString slaveHref_1, CString slaveHref_2, int nTimeout)
{
	bool bPostSuccess = false;
	CStringA strResult = doPost2(href, nTimeout, bPostSuccess);
	if (!bPostSuccess && _T("") != slaveHref)
	{
		strResult = doPost2(slaveHref, nTimeout, bPostSuccess);
	}
	if (!bPostSuccess && _T("") != slaveHref_1)
	{
		strResult = doPost2(slaveHref_1, nTimeout, bPostSuccess);
	}
	if (!bPostSuccess && _T("") != slaveHref_2)
	{
		strResult = doPost2(slaveHref_2, nTimeout, bPostSuccess);
	}

	return strResult;
}

CStringA CHttpGetPost::doPost2(CString   href, int nTimeout, bool &bPostSuccess)
{
	//return _T("");
	CStringA strResult = "";
	SOCKET hSocket = INVALID_SOCKET;
	SOCKADDR_IN servaddr;
	WORD vertion;
	WSADATA wsaData;
	CString strServerName = _T("");
	CString strObject = _T("");
	CString cstrIP = _T("");
	INTERNET_PORT nPort;
	int nRet = 0;
	char szPost[4096] = { 0 };
	bPostSuccess = false;

	DWORD dwServiceType;
	href.Replace(_T(" "), _T("%20"));
	CString cstrLogHref = href;
	cstrLogHref.Replace(_T("%"), _T("%%"));

	vertion = MAKEWORD(2, 2);
	WSAStartup(vertion, &wsaData);
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == hSocket)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 socket error, href:%s."), cstrLogHref.GetBuffer(0));
		return strResult;
	}

	AfxParseURL((LPCTSTR)href, dwServiceType, strServerName, strObject, nPort);

	//如果是域名，转化为IP
	cstrIP = strServerName;
	if (NULL != _tcsstr(strServerName, _T(".com")))
	{
		strServerName.Replace(_T("http://"), _T(""));
		strServerName.Replace(_T("/"), _T(""));
		strServerName.Replace(_T("www."), _T(""));
		if (0 != GetHostIpByDomain(strServerName, cstrIP))
		{
			Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 GetHostIpByDomain error, strServerName:%s, href:%s."), strServerName.GetBuffer(0), cstrLogHref.GetBuffer(0));
			strServerName = cstrIP = _T("23.91.96.161");
		}
	}

	CharCode szServerIP(cstrIP.GetBuffer(0));
	CharCode szServerName(strServerName.GetBuffer(0));
	servaddr.sin_addr.S_un.S_addr = inet_addr(szServerIP.GetStringA());
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(nPort);

	nRet = connect(hSocket, (SOCKADDR*)&servaddr, sizeof(SOCKADDR));
	if (nRet == SOCKET_ERROR)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 connect error, href:%s,errno:%d."), cstrLogHref.GetBuffer(0),errno);  // adding by 0912
		closesocket(hSocket);
		return strResult;
	}

	char szTableLen[10] = { 0 };
	sprintf(szTableLen, "%d", m_strTable.GetLength());
	CharCode szObject(strObject.GetBuffer(0));
	strcat(szPost, "POST ");
	strcat(szPost, szObject.GetStringA());
	strcat(szPost, " HTTP/1.1\r\n");
	strcat(szPost, "Content-Type: application/x-www-form-urlencoded\r\n");
	strcat(szPost, "User-Agent: eClass\r\n");
	strcat(szPost, "Accept: */*\r\n");
	strcat(szPost, "Host: ");
	strcat(szPost, szServerName.GetStringA());
	strcat(szPost, "\r\n");
	strcat(szPost, "Content-Length: ");
	strcat(szPost, szTableLen);
	strcat(szPost, "\r\n");
	strcat(szPost, "Cache-Control: no-cache\r\n");
	strcat(szPost, "\r\n");
	strcat(szPost, m_strTable);
	strcat(szPost, "\r\n\r\n");

	nRet = send(hSocket, szPost, strlen(szPost), 0);
	if (nRet == SOCKET_ERROR)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 send error, href:%s."), cstrLogHref.GetBuffer(0));
		closesocket(hSocket);
		WSACleanup();
		return strResult;
	}

	int nTimeStart = 0;
	int nSleepMSec = 10;
	char szRecvBuf[4096] = { 0 };
	CStringA strRecvDate = "";
	SetNonBlocking(hSocket, true);
	nTimeout = (0 < nTimeout) ? nTimeout : 5000;
	for (nTimeStart = 0; nTimeStart < nTimeout / nSleepMSec; nTimeStart++)
	{
		memset(szRecvBuf, 0, sizeof(szRecvBuf));
		nRet = recv(hSocket, szRecvBuf, sizeof(szRecvBuf)-1, 0);
		if (nRet <= SOCKET_ERROR)
		{
			//ERROR_INVALID_HANDLE这个错误有时会出现
			if (WSAEWOULDBLOCK == WSAGetLastError() || ERROR_INVALID_HANDLE == WSAGetLastError())
			{
				Sleep(nSleepMSec);
				continue;
			}
			else
			{
				Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 recv error, href:%s.WSAGetLastError:%0x"), cstrLogHref.GetBuffer(0), WSAGetLastError());
				break;
			}
		}
		else if (nRet == 0)
		{
			Sleep(10);
			continue;
		}
		else if (nRet > 0)
		{
			strRecvDate += szRecvBuf;

			//处理404错误
			CStringA strNotFound = "HTTP/1.1 404 Not Found";
			if (-1 != strRecvDate.Find(strNotFound))
			{
				strResult = strNotFound;
				Write_LogW(LOG_ERROR, _T("CHttpGetPost: doPost2 404 not find."));
				break;
			}

			CStringA strConLen = "Content-Length: ";
			int nLengthPos = strRecvDate.Find(strConLen);
			if (-1 == nLengthPos)
			{
				strResult = strRecvDate; //请求返错，把错误返回出去，用于区分网络连不上还是请求失败
				Write_LogW(LOG_ERROR, _T("CHttpGetPost: doPost2 not find Content-Length"));
				break;
			}
			int nLengthEnd = strRecvDate.Find("\r\n", nLengthPos);
			if (-1 == nLengthEnd)
			{
				strResult = strRecvDate; //请求返错，把错误返回出去，用于区分网络连不上还是请求失败
				Write_LogW(LOG_ERROR, _T("CHttpGetPost: doPost2 not find \r\n"));
				break;
			}
			//取出正文长度比较，如果不够则继续收
			CStringA strLen = strRecvDate.Mid(nLengthPos + strConLen.GetLength(), nLengthEnd - (nLengthPos + strConLen.GetLength()));
			int nPos = strRecvDate.Find("\r\n\r\n");
			if (-1 == nPos)
			{
				strResult = strRecvDate; //请求返错，把错误返回出去，用于区分网络连不上还是请求失败
				Write_LogW(LOG_ERROR, _T("CHttpGetPost: doPost2 not find \r\n\r\n"));
				break;
			}
			strResult = strRecvDate.Mid(nPos + 4);
			if (atoi(strLen.GetBuffer(0)) == strResult.GetLength())
			{
				//有可能返回的length字段为0
				if ("" == strResult)
				{
					strResult = "content-length error";
					Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 result content-length error, href:%s"), cstrLogHref.GetBuffer(0));
				}
				bPostSuccess = true;
				break;
			}
		}
	}

	//请求超时(用于区别无网络连接)
	if (nTimeout / nSleepMSec <= nTimeStart)
	{
		strResult = "ReqTimeout";
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2 request timeout, href:%s"), cstrLogHref.GetBuffer(0));
	}

	closesocket(hSocket);
	WSACleanup();
	return strResult;
}

CStringA CHttpGetPost::doPost2(CString   href, CString strFilePath, CString strFileName)
{
	DWORD dwServiceType;
	CStringA strResult = "";
	SOCKET hSocket = INVALID_SOCKET;
	SOCKADDR_IN servaddr;
	WORD vertion;
	WSADATA wsaData;
	CString strServerName = _T("");
	CString strObject = _T("");
	CString cstrIP = _T("");
	INTERNET_PORT nPort;
	int nRet = 0;
	int nFileLen = 0;
	char szPost[4096] = { 0 };
	char szContent[4096] = { 0 };
	char szTail[128] = { 0 };

	if (_T("") == href || _T("") == strFilePath || _T("") == strFileName)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) prama error."));
		return strResult;
	}

	href.Replace(_T(" "), _T("%20"));
	AfxParseURL((LPCTSTR)href, dwServiceType, strServerName, strObject, nPort);
	CString cstrLogHref = href;
	cstrLogHref.Replace(_T("%"), _T("%%"));

	//读取文件
	//string strAFilePath = CW2A(strFilePath.GetBuffer(0), CP_UTF8);
	CharCode szFilePath(strFilePath.GetBuffer(0));
	FILE *fp = fopen(szFilePath.GetStringA(), "rb");
	if (fp == NULL)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) fopen error, href:%s, filePath:%s, errno:%d."), cstrLogHref.GetBuffer(0), strFilePath.GetBuffer(0), WSAGetLastError());
		return strResult;
	}

	fseek(fp, 0, SEEK_END);
	nFileLen = ftell(fp);
	rewind(fp);
	Write_LogW(LOG_INFO, _T("CHttpGetPost::DoPost2(file) file:%s, fileLen:%d."), strFileName.GetBuffer(0), nFileLen);

	char *fBuffer = new char[nFileLen + 1];
	memset(fBuffer, 0, nFileLen + 1);
	fseek(fp, 0, SEEK_SET);
	fread(fBuffer, sizeof(char), nFileLen, fp);
	fclose(fp);

	//创建socket并连接
	vertion = MAKEWORD(2, 2);
	WSAStartup(vertion, &wsaData);
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == hSocket)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) socket error, href:%s."), cstrLogHref.GetBuffer(0));
		return strResult;
	}

	//如果是域名，转化为IP
	cstrIP = strServerName;
	if (NULL != _tcsstr(strServerName, _T(".com")))
	{
		strServerName.Replace(_T("http://"), _T(""));
		strServerName.Replace(_T("/"), _T(""));
		strServerName.Replace(_T("www."), _T(""));
		if (0 != GetHostIpByDomain(strServerName, cstrIP))
		{
			Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) GetHostIpByDomain error, href:%s."), cstrLogHref.GetBuffer(0));
			strServerName = cstrIP = _T("23.91.96.161");
		}
	}

	CharCode szServerIP(cstrIP.GetBuffer(0));
	CharCode szServerName(strServerName.GetBuffer(0));
	servaddr.sin_addr.S_un.S_addr = inet_addr(szServerIP.GetStringA());
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(nPort);

	nRet = connect(hSocket, (SOCKADDR*)&servaddr, sizeof(SOCKADDR));
	if (nRet == SOCKET_ERROR)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) connect error, href:%s."), cstrLogHref.GetBuffer(0));
		closesocket(hSocket);
		return strResult;
	}

	string strBoundary = "----WebKitFormBoundaryCJsAP52jsafp27FY";
	string strEndline = "--" + strBoundary + "--\r\n";// 数据结束标志

	CharCode szObject(strObject.GetBuffer(0));
	strcat(szPost, "POST ");
	strcat(szPost, szObject.GetStringA());
	strcat(szPost, " HTTP/1.1\r\n");
	strcat(szPost, "Host: ");
	strcat(szPost, szServerName.GetStringA());
	strcat(szPost, "\r\n");
	strcat(szPost, "User-Agent: eClass\r\n");
	strcat(szPost, "Connection: Keep-Alive\r\n");
	strcat(szPost, "Accept: */*\r\n");
	strcat(szPost, "Pragma: no-cache\r\n");
	strcat(szPost, "Cache-Control: no-cache\r\n");
	strcat(szPost, "Content-Type: multipart/form-data; boundary=");
	strcat(szPost, strBoundary.c_str());
	strcat(szPost, "\r\n");

	strcat(szContent, "--");
	strcat(szContent, strBoundary.c_str());
	strcat(szContent, "\r\n");
	strcat(szContent, "Content-Disposition: form-data; name=\"FileName\"");
	strcat(szContent, "\r\n\r\n\"");
	CharCode szFilename(strFileName.GetBuffer(0));
	strcat(szContent, szFilename.GetStringA());
	strcat(szContent, "\"\r\n");

	strcat(szContent, "--");
	strcat(szContent, strBoundary.c_str());
	strcat(szContent, "\r\n");
	strcat(szContent, "Content-Disposition: form-data; name=\"file\"; filename=\"");
	strcat(szContent, szFilename.GetStringA());
	strcat(szContent, "\"\r\n");
	strcat(szContent, "Content-Type: application/octet-stream");
	strcat(szContent, "\r\n\r\n");

	strcat(szTail, strEndline.c_str());
	//注意下面这个参数Content-Length，这个参数值是：http请求头长度+请求尾长度+文件总长度   
	char temp[64] = { 0 }; 
	sprintf(temp, "Content-Length: %d\r\n\r\n", strlen(szContent) + strlen(szTail) + nFileLen);
	strcat(szPost, temp);
 
	//发送post头
	CStringA strAPostHeader = szPost;
	strAPostHeader += szContent;
	nRet = send(hSocket, strAPostHeader.GetBuffer(0), strAPostHeader.GetLength(), 0);
	if (nRet == SOCKET_ERROR)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) send error, href:%s."), cstrLogHref.GetBuffer(0));
		closesocket(hSocket);
		WSACleanup();
		return strResult;
	}

	char sendBuff[4096];
	int nSendSize = 0;
	while (nSendSize < nFileLen)
	{
		int nChunkSize = (sizeof(sendBuff) <= nFileLen - nSendSize) ? sizeof(sendBuff) : (nFileLen - nSendSize);
		memcpy(sendBuff, fBuffer + nSendSize, nChunkSize);
		nRet = send(hSocket, strAPostHeader.GetBuffer(0), nChunkSize, 0);
		if (nRet == SOCKET_ERROR)
		{
			Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) send error, startPos:%d, chunkSize:%d.href:%s."), nSendSize, nChunkSize, cstrLogHref.GetBuffer(0));
			closesocket(hSocket);
			WSACleanup();
			return strResult;
		}
		nSendSize += nChunkSize;
		Sleep(20);
	}

	nRet = send(hSocket, szTail, strlen(szTail), 0);
	if (nRet == SOCKET_ERROR)
	{
		Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) send tail error, href:%s."), cstrLogHref.GetBuffer(0));
		closesocket(hSocket);
		WSACleanup();
		return strResult;
	}

	char szRecvBuffer[4096] = { 0 };
	while (true)
	{
		//SetNonBlocking(hSocket, true);
		int nRet = recv(hSocket, szRecvBuffer, sizeof(szRecvBuffer), 0);
		if (nRet == 0 || nRet == WSAECONNRESET)
		{
			break;
		}
		else if (nRet == SOCKET_ERROR)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError() || ERROR_INVALID_HANDLE == WSAGetLastError())
			{
				Sleep(10);
				continue;
			}
			else
			{
				Write_LogW(LOG_ERROR, _T("CHttpGetPost::DoPost2(file) recv error, href:%s.WSAGetLastError:%0x"), cstrLogHref.GetBuffer(0), WSAGetLastError());
				break;
			}
		}
		else
		{
			strResult = szRecvBuffer;
			int nLengthEnd = strResult.Find("\r\n\r\n");
			if (-1 == nLengthEnd)
			{
				strResult = "";
				Write_LogW(LOG_ERROR, _T("CHttpGetPost: doPost2(file) not find \r\n\r\n"));
				break;
			}
			//取出正文长度比较，如果不够则继续收
			strResult = strResult.Mid(nLengthEnd+4);
			Write_LogW(LOG_INFO, _T("CHttpGetPost::DoPost2(file) recv success, href:%s, szRecvBuffer:%s."), cstrLogHref.GetBuffer(0), szRecvBuffer);
			break;
		}
	}
	
	closesocket(hSocket);
	WSACleanup();
	return strResult;
}