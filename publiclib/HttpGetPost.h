#ifndef __HTTPGETPOST_H__
#define __HTTPGETPOST_H__

#include "def.h"
#include "CharCode.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

bool SetNonBlocking(SOCKET hSocket, bool bNB);

class EXPORT_FUNCTION CHttpGetPost
{
public:
	void AddParam(CStringA strKey, CStringA strValue);
	void AddParam(CStringA strKey, int nValue);
	CString doPost(CString href);
	CStringA doPost2(CString href, CString slaveHref = _T(""), CString slaveHref_1 = _T(""), CString slaveHref_2 = _T(""), int nTimeout = 5000);
	CStringA doPost2(CString href, CString strFilePath, CString strFileName);
	CString doGet(CString href);
	char *doGet(char*href);
	CString doPost(CString href, CString strFilePath, CString strFileName, CString strVersion, CString strGroupID, CString strUserID, CString strRoomID, CString strVerificationKey);
	CString PreFileData(CString strBoundary, CString strFileName, CString strVersion, CString strGroupID, CString strUserID, CString strRoomID, CString strVerificationKey);
	CString EndFileData(CString strBoundary);
	CString MakeRequestHeaders(CString &strBoundary);
	CHttpGetPost();
	virtual ~CHttpGetPost();
private:
	CStringA doPost2(CString   href, int nTimeout, bool &bPostSuccess);
private:
	CStringA m_strTable;
};

#endif