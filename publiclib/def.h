#ifndef __MY_DEF__
#define __MY_DEF__

#define EXPORT_FUNCTION AFX_EXT_CLASS

#include "stdafx.h"
#include <list>
#include <map>
using namespace std;


//机构
#define ORGANIZATION_TALK915
//#define ORGANIZATION_CHUNXI
//#define ORGANIZATION_TALKILLA

//------------------------------------模块间结构定义------------------------------------

#define GETROOMSTATUS	(WM_USER + 2000)  //获取服务器教室状态，用于通知各模块窗口处理

//用户类型定义(暂时不用)
typedef enum _tagUserTypeEnum
{
	USER_TYPE_TEACHER = 1,
	USER_TYPE_STUDENT,
	USER_TYPE_ASSISTANT, //助教
	USER_TYPE_WATCHER, //巡视员
	USER_TYPE_GUEST = 5  //游客
}UserTypeEnum;

//角色类型定义
typedef enum _tagRoleTypeEnum
{
	ROLE_TYPE_HOST = 1,  //主讲
	ROLE_TYPE_ATTENDER,  //参与者
	ROLE_TYPE_ASSISTANT, //助教
	ROLE_TYPE_WATCHER,   //巡视员
	ROLE_TYPE_GUEST = 5  //游客
}RoleTypeEnum;

//教室类型定义
typedef enum _tagRoomTypeEnum
{
	ROOM_TYPE_1VS1 = 1, //1对1
	ROOM_TYPE_SMALL,     //小班课
	ROOM_TYPE_BIG,     //大班课
	ROOM_TYPE_MAX
}RoomTypeEnum;

//statusBar显示滚动消息
typedef struct _tagXmlScrollMessage
{
	string messageType;
	string messageTextSC; //显示内容(中文)
	string messageTextEN; //显示内容(英文)
	string hyperlink;  //web链接
}XmlScrollMessage;

typedef list<XmlScrollMessage *> P_LIST_XMLSCROLLMESSAGE;

//教室状态
typedef struct _tagRoomStatus
{
	string strTimerStartTime; //课程计时器开始计时的时间            
	bool bLockWhiteBoard;   //是否锁定白板，为0时锁定全部学生白板
	list<int> listLimitVideo;  //关闭视频学生id
	list<int> listAllowMic;    //关闭麦克风学生id
	list<int> listLimitTalk;   //禁言学生id
	list<int> listLimitEnter;  //请出教室学生id   <先不实现>
	int nTempHost;          //临时主讲id
	map<int, int> mapLike;  //点赞(userID, count)
}RoomStatus;

//教室状态类型
typedef enum _tagRoomStatusEnum
{
	ROOM_STATUS_STRAT_TIMER_TIME, 
	ROOM_STATUS_LOCK_WHITEBOARD,
	ROOM_STATUS_LIMIT_VIDEO,
	ROOM_STATUS_LIMIT_MIC,
	ROOM_STATUS_LIMIT_TALK,
	ROOM_STATUS_LIMIT_ENTER,
	ROOM_STATUS_TEMP_HOST, //临时主讲
	ROOM_STATUS_LIKE //点赞
}RoomStatusEnum;

enum LOGIN_MODE
{
	MODE_CLIENT_USER = 0,//客户端登录的用户
	MODE_CLIENT_GUEST,//客户端登录的游客
	MODE_WEB_USER,//web登录的用户
	MODE_MAX
};


#ifdef PUBLICLIB_EXPORT
#define PUBLICLIB_API extern "C" __declspec(dllexport)
#else
#define PUBLICLIB_API extern "C" __declspec(dllimport)
#endif

PUBLICLIB_API TCHAR g_szUploadDocServer[256];//doc server的ip
PUBLICLIB_API TCHAR g_szUploadServerIP[256];
PUBLICLIB_API TCHAR g_szUploadServerIPCDN[256];//通过域名转换后的IP
PUBLICLIB_API TCHAR g_szUploadServerDomain[256]; //doc下载域名
PUBLICLIB_API USHORT g_nUploadServerPort;//doc下载server端口号
PUBLICLIB_API LOGIN_MODE g_enumCurLoginMode;
PUBLICLIB_API TCHAR g_szCurGroupName[128];
PUBLICLIB_API TCHAR g_szeClassGroupName[128];
PUBLICLIB_API TCHAR g_szTGroupName[128];
#endif