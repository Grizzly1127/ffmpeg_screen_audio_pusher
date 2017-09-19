
#include "def.h"


/* ---------------------- 配置项 ---------------------- */
//[UploadServer]
TCHAR g_szUploadDocServer[256] = {0};//GetDocServer接口返回， http://106.75.129.178:8080/
TCHAR g_szUploadServerIP[256] = { 0 }; //服务器IP 106.75.129.178，用于post请求
TCHAR g_szUploadServerIPCDN[256] = {0};//106.75.129.178或者CDN转换后IP，用于文件下载
TCHAR g_szUploadServerDomain[256] = { 0 }; //域名eclassdoc1.talk915.com
USHORT g_nUploadServerPort = 8080; //文件下载端口

LOGIN_MODE g_enumCurLoginMode = MODE_MAX;//当前登录类型
TCHAR g_szCurGroupName[128] = {0}; //当前登录机构名（如rQXBUcWduZIQZCd）
TCHAR g_szeClassGroupName[128] = { 0 }; //eclass账户的机构名
TCHAR g_szTGroupName[128] = { 0 };	//T账户的机构名
