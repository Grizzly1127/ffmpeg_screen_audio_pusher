#ifndef ISCREENAUDIOPUSHER_H
#define ISCREENAUDIOPUSHER_H

#ifdef FFMPEG_SCREEN_AUDIO_PUSHER_EXPORTS
#define RECORDAPI __declspec(dllexport)
#else
#define RECORDAPI __declspec(dllimport)
#endif

#include "MediaTypeDef.h"
using namespace MediaType;
extern "C"
{
	RECORDAPI void* MR_CreateScreenAudioRecorder(); //创建推流对象

	RECORDAPI void MR_DestroyScreenAudioRecorder(void* pObject); //销毁该对象

	RECORDAPI int MR_SetRecordInfo(void* pObject, const RECORD_INFO& recordInfo); //设置推流参数

	RECORDAPI int MR_StartRecord(void* pObject); //开始推流

	RECORDAPI int MR_StopRecord(void* pObject); //停止推流
}

RECORDAPI DEVICE_INFO MR_GetDevice(void* pObject); //获取设备信息
#endif