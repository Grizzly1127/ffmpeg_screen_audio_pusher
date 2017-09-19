#ifndef ISCREENAUDIOCLASS_H
#define ISCREENAUDIOCLASS_H

#include "MediaTypeDef.h"
using namespace MediaType;
namespace MediaRtmpPusher
{
	class IScreenAudioClass
	{
	public:
		virtual int SetRecordInfo(const RECORD_INFO& recordInfo) = 0;
		virtual int StartRecord() = 0;
		virtual int StopRecord() = 0;
		virtual DEVICE_INFO GetDevice() = 0;

		virtual ~IScreenAudioClass(){}
	};

	IScreenAudioClass* CreateScreenAudioRecorder();
	void DestroyScreenAudioRecorder(IScreenAudioClass* pRecorder);
}

#endif