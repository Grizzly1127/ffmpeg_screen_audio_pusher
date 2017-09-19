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
	RECORDAPI void* MR_CreateScreenAudioRecorder();

	RECORDAPI void MR_DestroyScreenAudioRecorder(void* pObject);

	RECORDAPI int MR_SetRecordInfo(void* pObject, const RECORD_INFO& recordInfo);

	RECORDAPI int MR_StartRecord(void* pObject);

	RECORDAPI int MR_StopRecord(void* pObject);
}

RECORDAPI DEVICE_INFO MR_GetDevice(void* pObject);
#endif