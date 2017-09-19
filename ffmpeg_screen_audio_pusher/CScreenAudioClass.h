#pragma once
#include "IScreenAudioClass.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <math.h>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h" 
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"  
#include "libavutil/time.h"  
#include "libavdevice/avdevice.h" 
#include "inttypes.h"
#include "SDL.h"
#include "SDL_thread.h"
};
//DirectShow
#include "dshow.h"  //包含ICreateDevEnum
#include "MediaTypeDef.h"

#include "afxcmn.h"
#include "afxwin.h"

#define AUDIO_BUF_SIZE 1024
#define SAMPLE_ARRAY_SIZE (8 * 65536)


#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_AUDIO_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_VIDEO_REFRESH_EVENT (SDL_USEREVENT + 2)
#define FF_BREAK_EVENT (SDL_USEREVENT + 3)
#define FF_QUIT_EVENT    (SDL_USEREVENT + 10)

DEFINE_GUID(CLSID_PushSourceDesktop,
	0x4ea6930a, 0x2c8a, 0x4ae6, 0xa5, 0x61, 0x56, 0xe4, 0xb5, 0x4, 0x44, 0x39);

typedef struct AudioParams {
	int freq;
	int channels;
	int channel_layout;
	enum AVSampleFormat fmt;
} AudioParams;


enum ShowMode {
	SHOW_MODE_NONE = -1,
	SHOW_MODE_VIDEO = 0,
	SHOW_MODE_WAVES = 1,
	SHOW_MODE_RDFT = 2,
	SHOW_MODE_NB = 3
};

typedef struct stream_info{
	AVFormatContext		*m_pFormatCtx;
	SDL_Window			*m_pShowScreen;			//音视频显示SDL窗口
	SDL_Surface			*m_pScreenSurface;		//与screen绑定的变量
	int					 m_xLeft;				//显示窗体的坐标及大小
	int					 m_yTop;
	int					 m_width;
	int					 m_height;
	int					 m_iAbortRequest;		//退出标记
	int					 m_iRefresh;				//刷新标记
	int					 m_iShowMode;			//显示模式
	int					 m_iPaused;				//暂停标记
	SDL_Renderer		*m_pSdlRender;
	SDL_Texture			*m_pSdlTexture;
	/************************音频相关参数-start*********************/
	SDL_Thread			*m_pAudioThr;		//音频测试线程
	SDL_Thread			*m_pAudioRefreshThr;	//音频刷新线程句柄
	AVStream			*m_pAudioStream;		//音频流
	AVFrame				*m_pAudioFrame;		//音频帧
	AVAudioFifo			*m_pAudioFifo;
	SDL_mutex			*m_pAudioMutex;
	AudioParams			 m_AudioPrm;
	uint8_t				*m_pAudioBuf;
	int					 m_iAudioBufSize;
	int					 m_iAudioBufIndex;
	int					 m_iAduioPktSize;
	int					 m_iAudioWriteBufSize;
	int					 m_iAudioLastStart;
	uint8_t				 m_uSilenceBuf[AUDIO_BUF_SIZE];
	int16_t				 m_iSampleArray[SAMPLE_ARRAY_SIZE];
	int					 m_iSampleArrayIndex;
	/************************音频相关参数-end***********************/

	/************************视频相关参数-satrt*********************/
	SDL_Thread			*m_pVideoThr;			//视频测试线程
	SDL_Thread			*m_pVideoRefreshThr;		//视频刷新线程句柄
	AVFifoBuffer		*m_pVideoFifo;
	SDL_mutex			*m_pVideoMutex;
	AVStream			*m_pVideoStream;			//视频流
	uint8_t				*m_pPushPicSize;			//推送Pic大小
	SwsContext			*m_pVideoSwsCtx;			//视频变化ctx
	/************************视频相关参数-end***********************/
}struct_stream_info;

using namespace MediaType;
using namespace MediaRtmpPusher;
namespace MediaRtmpPusher
{
	class CScreenAudioRecord : public IScreenAudioClass
	{
	public:
		CScreenAudioRecord();
		~CScreenAudioRecord();

		int SetRecordInfo(const RECORD_INFO& recordInfo) override;
		int StartRecord() override;
		int StopRecord() override;
		DEVICE_INFO GetDevice() override;
	private:
		void RegisterPlugin();
		bool IsOsVersionVistaOrGreater();
		void ShellExecuteExOpen(CString appName, CString appPath);
		void InitData();
		void InitSdl();	
		int CheckRecordInfo();
		int CreateVideoWindow();
		int OpenCamera();
		int InitVideoWindow();
		int OpenAduio();
		int OpenRtmpAddr();
		int OpenRtmpUrl();
		int UpdateSdlInfo();
		int  GetDeviceInfo(int _iDeviceType);
		CString GetWorkDir();

	public:
		struct_stream_info*						m_pStreamInfo;	//音视频全局结构体
		RECORD_INFO								m_recordInfo;	//视频设置
		AVFormatContext						   *m_pFmtVideoCtx;	//视频采集format
		AVFormatContext						   *m_pFmtAudioCtx;	//音频采集format
		AVFormatContext						   *m_pFmtRtmpCtx;	//rtmp推送format
		AVCodecContext						   *m_pCodecVideoCtx;//视频采集解码器信息
		AVCodecContext						   *m_pCodecAudioCtx;//音频采集解码器信息
		int										m_iVideoIndex;	//视频采集解码器索引
		int										m_iAudioIndex;	//音频采集解码器索引
		int										m_iVideoOutIndex;//推送视频解码器索引
		int										m_iAudioOutIndex;//推送音频解码器索引
		int										m_iSrcVideoHeight;//源视频高
		int										m_iSrcVideoWidth;//源视频宽
		int										m_iWindowWidth;	//背景板的宽
		int										m_iWindowHeight;	//背景板的高
		BOOL									m_blCreateVideoWin;//是否创建视频窗口
		BOOL									m_blPushSuccess;//是否推送成功
		BOOL									m_bPushStatus;//推流状态

		SDL_Thread							   *m_pPushStreamThrid;//推流线程
		SDL_Thread							   *m_pWriteStreamThrid;//推流线程

		DEVICE_INFO								m_mapDeviceInfo;	//设备信息容器

	};

	int video_thr(LPVOID lpParam);
	int audio_thr(LPVOID lpParam);
	int push_thr(LPVOID lpParam);
	int write_frame_thr(LPVOID lpParam);
	void set_packet(AVPacket* pkt);
	AVPacket* get_packet();

	list<AVPacket*> m_listPacket;
	CritSec m_CritSec;
}
