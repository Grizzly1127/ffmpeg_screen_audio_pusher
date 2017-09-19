#ifndef MEDIATYPEDEF_H
#define MEDIATYPEDEF_H
#include <CString>
#include <string>
#include <map>
#include <vector>
#include <comdef.h>

namespace MediaType
{
	struct RECORD_INFO
	{
		char rtmp_url[1024];//推流地址
		char video_device_name[1024];//视频设备名
		char audio_device_name[1024];//音频设备名
		int  video_dst_width;//视频宽
		int  video_dst_height;//视频高
		int  video_frame_rate;//帧率
		HWND preview_hwnd;//视频显示句柄
		RECORD_INFO()
		{
			memset(rtmp_url, 0, 1024);
			memset(video_device_name, 0, 1024);
			memset(audio_device_name, 0, 1024);
			video_dst_width = 0;
			video_dst_height = 0;
			video_frame_rate = 0;
			preview_hwnd = NULL;
		}
	};

	enum DeviceType{
		n_Video = 0,	//视频
		n_Audio = 1		//音频
	};

	typedef map<int, std::vector<std::string>> DEVICE_INFO;
}
#endif