/************************************
*comment:日志输出到文件
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __LOG_EX_H__
#define __LOG_EX_H__

#include <stdarg.h>
#include "def.h"

#define LOG_INFO  1 //基本信息
#define LOG_ERROR 2 //错误信息

#define _LOG_WRITE_STATE_  //日志编译开关
#define LOG_LEVEL 1 //日志输出级别>=LOG_LEVEL



#define LOG_MAXTEXT 2048//一行日志的最大长度

bool EXPORT_FUNCTION Write_Log(int nLogType, const char *pstrFmt, ...);
bool EXPORT_FUNCTION Write_LogW(int nLogType, TCHAR *pstrFmt, ...);

bool EXPORT_FUNCTION Write_Data(int nLogType, const char *pstrFmt, ...);

#endif
