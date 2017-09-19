/************************************
*comment:日期、时间相关函数
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __DATETIME_EX_H__
#define __DATETIME_EX_H__

#include "def.h"
#include <time.h>

//EXPORT_FUNCTION char* Ex_GetDateTime();
//EXPORT_FUNCTION long Ex_GetDateTimeSecond();
//EXPORT_FUNCTION char* Ex_GetDate();
//EXPORT_FUNCTION char* Ex_GetTime();
//EXPORT_FUNCTION int Ex_GetHour();
//EXPORT_FUNCTION long TimeSwitthToSecond(const char* cszDateTime);
//EXPORT_FUNCTION const char* TimeSwitchToString(long lDateTime);


#define DATE_TIME_BUF_LEN 128
class EXPORT_FUNCTION Ex_DateTime
{
public:
	Ex_DateTime();
	~Ex_DateTime();

public:
	 char* Ex_GetDateTime();
	 long Ex_GetDateTimeSecond();
	 char* Ex_GetDate();
	 char* Ex_GetTime();
	 int Ex_GetHour();
	 long TimeSwitthToSecond(const char* cszDateTime);
	 const char* TimeSwitchToString(time_t lDateTime);

private:
	char *m_PDateTime;
};

#endif