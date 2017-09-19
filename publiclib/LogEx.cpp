#include "stdafx.h"
#include "CharCode.h"
#include "LogEx.h"
#include "DirFileEx.h"
#include "DateTimeEx.h"

char g_szLogPath[512] = { 0};//

class LogInit
{
public:
	LogInit();
	~LogInit();
};

LogInit::LogInit()
{
	CharCode codePath(GetRunDirW());
	sprintf(g_szLogPath, "%s", codePath.GetStringA());
	Ex_CreateDiretory(g_szLogPath);
}

LogInit::~LogInit()
{
}
static LogInit g_LogInit;

/*********************************************************************
* 函数名称:void Write_Log(int nLogType, char *pstrFmt, ...)
* 说明:日志输出函数，支持变长参数
* 调用者：任何需要写日志的地方
* 输入参数:
* int nLogTypee --  日志类别, LOG_INFO 表示基本信息,LOG_ERROR 表示错误信息
* char *pstrFmt  --  日志内容
* ...            --  变长参数
* 输出参数：
* 无
* 返回值:
* void  --
* use example:
* Write_Log(0,"%s %d %s", __FILE__, __LINE__, "LogContent");  //only need to call this function to print log
* 注意要打印的日志长度不能超过LOG_MAXTEXT个字节
*********************************************************************/
bool EXPORT_FUNCTION Write_Log(int nLogType, const char *pstrFmt, ...)
{
#ifdef _LOG_WRITE_STATE_ 
	if (nLogType != LOG_INFO && nLogType != LOG_ERROR)//堆栈被破坏，参数异常，nLogType变为随机值
		return false;

	if (pstrFmt == NULL)
		return false;
	//取出格式化输入的字符串
	char szLogBuf[LOG_MAXTEXT] = { 0 };
	va_list vArgList;
	va_start(vArgList, pstrFmt);
	_vsnprintf_s(szLogBuf, LOG_MAXTEXT, LOG_MAXTEXT-1, pstrFmt, vArgList);
	va_end(vArgList);

	//日志文件路径+文件名
	char szLogPathFile[512] = { 0 };
	//char szData[DATE_TIME_BUF_LEN] = { 0 };
	Ex_DateTime dateTime;
	sprintf_s(szLogPathFile, 512, "%s%s.log", g_szLogPath, dateTime.Ex_GetDate());

	//日志一行的内容
	char szLogText[LOG_MAXTEXT + 50] = { 0 };

	//追加方式打开文件
	FILE *pFile = NULL;
	pFile = fopen(szLogPathFile, "a+");
	if (NULL == pFile)
	{
		return false;
	}

	//打印线程ID
	unsigned long lThreadID = GetCurrentThreadId();

	//char szDataTime[32]={0};
	//strcpy(szDataTime, dateTime.Ex_GetDateTime());
	if (nLogType == LOG_INFO)
	{
		sprintf_s(szLogText, LOG_MAXTEXT + 50, "%s INFO[%08x] %s\r\n", dateTime.Ex_GetDateTime(), lThreadID, szLogBuf);
	}
	else if (nLogType == LOG_ERROR)
	{
		sprintf_s(szLogText, LOG_MAXTEXT + 50, "%s ERROR[%08x] %s\r\n", dateTime.Ex_GetDateTime(), lThreadID, szLogBuf);
	}

	//printf(szLogText);

	fwrite(szLogText, strlen(szLogText), 1, pFile);
	fclose(pFile);

	return true;
#endif _LOG_WRITE_STATE_
}


bool EXPORT_FUNCTION Write_Data(int nLogType, const char *pstrFmt, ...)
{
#ifdef _LOG_WRITE_STATE_ 

	//取出格式化输入的字符串
	char szLogBuf[LOG_MAXTEXT] = { 0 };
	va_list vArgList;
	va_start(vArgList, pstrFmt);
	_vsnprintf_s(szLogBuf, LOG_MAXTEXT, LOG_MAXTEXT-1, pstrFmt, vArgList);
	va_end(vArgList);

	//日志文件路径+文件名
	char szLogPathFile[512] = { 0 };
	Ex_DateTime dateTime;
	sprintf_s(szLogPathFile, 512, "%s%s.data", g_szLogPath, dateTime.Ex_GetDate());

	//日志一行的内容
	char szLogText[LOG_MAXTEXT + 50] = { 0 };

	//追加方式打开文件
	FILE *pFile = NULL;
	pFile = fopen(szLogPathFile, "a+");
	if (NULL == pFile)
	{
		return false;
	}

	if (nLogType == LOG_INFO)
	{
		sprintf_s(szLogText, LOG_MAXTEXT + 50, "%s INFO %s\r\n", dateTime.Ex_GetDateTime(), szLogBuf);
	}
	else if (nLogType == LOG_ERROR)
	{
		sprintf_s(szLogText, LOG_MAXTEXT + 50, "%s ERROR %s\r\n", dateTime.Ex_GetDateTime(), szLogBuf);
	}

	printf(szLogText);

	fwrite(szLogText, strlen(szLogText), 1, pFile);
	fclose(pFile);

	return true;
#endif _LOG_WRITE_STATE_
}

/*
Write_LogW(LOG_INFO, _T("%s---%s"), CString.GetBuffer(0), CString.GetBuffer(0));//正确用法
Write_LogW(LOG_INFO, _T("%s---%s"), CA2W(...), CA2W(...); //错误用法1：这种有时会出现莫名的错误
Write_LogW(LOG_INFO, _T("%s---%s"), CString, CString);//错误用法2
Write_Log(LOG_INFO, "%s---%s", string, string);//错误用法3
*/
bool EXPORT_FUNCTION Write_LogW(int nLogType, TCHAR *pstrFmt, ...)
{
	if (nLogType != LOG_INFO && nLogType != LOG_ERROR)//堆栈被破坏，参数异常，nLogType变为随机值
		return false;

	if (pstrFmt == NULL)
		return false;

	int nSize = 0;
	TCHAR wcharLogBuf[LOG_MAXTEXT] = { 0 };


	va_list args;
	va_start(args, pstrFmt);	
	_vsnwprintf(wcharLogBuf, LOG_MAXTEXT, pstrFmt, args);

	va_end(args);

	//wcharLogBuf中带%s,%d等会导致Write_Log崩溃,需替换
	CString cstrLogBuf = wcharLogBuf;
	cstrLogBuf.Replace(_T("%"), _T("%%"));

	CharCode charLog(cstrLogBuf.GetBuffer(0));
	Write_Log(nLogType, charLog.GetStringA());

	return true;
}