#include "stdafx.h"
#include "IScreenAudioPusher.h"
#include "IScreenAudioClass.h"
#include "strsafe.h"
#include <string>
#include <map>
#include <vector>
#include <comdef.h>

using namespace MediaRtmpPusher;
using namespace MediaType;
RECORDAPI void* MR_CreateScreenAudioRecorder()
{
	return (void*)CreateScreenAudioRecorder();
}


RECORDAPI void MR_DestroyScreenAudioRecorder(void* pObject)
{
	DestroyScreenAudioRecorder((IScreenAudioClass*)pObject);
}

RECORDAPI DEVICE_INFO MR_GetDevice(void* pObject)
{
	DEVICE_INFO mapDeviceInfo;
	mapDeviceInfo.clear();
	IScreenAudioClass* pRecorder = (IScreenAudioClass*)pObject;
	if (pRecorder){
		mapDeviceInfo = pRecorder->GetDevice();
	}
	return mapDeviceInfo;
}

RECORDAPI int MR_SetRecordInfo(void* pObject, const RECORD_INFO& recordInfo)
{
	IScreenAudioClass* pRecorder = (IScreenAudioClass*)pObject;
	if (pRecorder)
	{
		return pRecorder->SetRecordInfo(recordInfo);
	}
	return -1;
}


RECORDAPI int MR_StartRecord(void* pObject)
{
	IScreenAudioClass* pRecorder = (IScreenAudioClass*)pObject;
	if (pRecorder)
	{
		return pRecorder->StartRecord();
	}
	return -1;
}


RECORDAPI int MR_StopRecord(void* pObject)
{
	IScreenAudioClass* pRecorder = (IScreenAudioClass*)pObject;
	if (pRecorder)
	{
		return pRecorder->StopRecord();
	}
	return -1;
}
