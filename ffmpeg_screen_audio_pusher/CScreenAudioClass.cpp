#pragma once
#include "stdafx.h"
#include "CScreenAudioClass.h"

namespace MediaRtmpPusher
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	CScreenAudioRecord::CScreenAudioRecord()
	{
		m_iVideoIndex = -1;
		m_iAudioIndex = -1;
		m_iVideoOutIndex = -1;
		m_iAudioOutIndex = -1;
		m_iSrcVideoHeight = -1;
		m_iSrcVideoWidth = -1;
		m_iWindowWidth = -1;
		m_iWindowHeight = -1;

		m_blPushSuccess = FALSE;
		m_bPushStatus = FALSE;
		m_blCreateVideoWin = FALSE;

		m_pPushStreamThrid = NULL;
		m_pWriteStreamThrid = NULL;

		m_pStreamInfo = NULL;
		m_pFmtRtmpCtx = NULL;
		m_pFmtVideoCtx = NULL;
		m_pFmtAudioCtx = NULL;
		m_pCodecVideoCtx = NULL;
		m_pCodecAudioCtx = NULL;

		RegisterPlugin();

		//初始化网络库
		av_register_all();
		avformat_network_init();
		avdevice_register_all();

		InitData();
		InitSdl();
	}

	CScreenAudioRecord::~CScreenAudioRecord()
	{
		
	}

	void CScreenAudioRecord::RegisterPlugin()
	{
		CString strWorkPath = GetWorkDir();
		CString strAppComAudio = _T("");
		CString strAppComScreen = _T("");
		strAppComAudio.Format(_T("/C regsvr32 /s %s\\audio_sniffer.dll"), strWorkPath);
		strAppComScreen.Format(_T("/C regsvr32 /s %s\\screen-capture-recorder.dll"), strWorkPath);

		ShellExecute(NULL, _T("open"), _T("cmd.exe"), strAppComAudio, NULL, SW_HIDE);
		ShellExecute(NULL, _T("open"), _T("cmd.exe"), strAppComScreen, NULL, SW_HIDE);
	}


	void CScreenAudioRecord::InitData()
	{
		m_pStreamInfo = (struct_stream_info *)calloc(1, sizeof(struct_stream_info));
		if (NULL == m_pStreamInfo)
		{
			return;
		}

		//初始化流媒体
		m_pStreamInfo->m_pFormatCtx = avformat_alloc_context();
		if (NULL == m_pStreamInfo->m_pFormatCtx){
			Write_Log(LOG_ERROR, "InitData : avformat_alloc_context error!");
			return;
		}

		m_pStreamInfo->m_pAudioThr = NULL;
		m_pStreamInfo->m_pVideoThr = NULL;
		m_pStreamInfo->m_pShowScreen = NULL;
		m_pStreamInfo->m_pScreenSurface = NULL;
		m_pStreamInfo->m_xLeft = 0;
		m_pStreamInfo->m_yTop = 0;
		m_pStreamInfo->m_width = 0;
		m_pStreamInfo->m_height = 0;
		m_pStreamInfo->m_pAudioStream = NULL;
		m_pStreamInfo->m_pAudioFifo = NULL;
		m_pStreamInfo->m_pVideoFifo = NULL;
		m_pStreamInfo->m_pAudioMutex = SDL_CreateMutex();
		m_pStreamInfo->m_pVideoMutex = SDL_CreateMutex();
		m_pStreamInfo->m_pVideoStream = NULL;
		m_pStreamInfo->m_pAudioFrame = NULL;
		m_pStreamInfo->m_pAudioBuf = NULL;
		m_pStreamInfo->m_iAudioBufSize = 0;
		m_pStreamInfo->m_iAudioBufIndex = 0;
		m_pStreamInfo->m_iAduioPktSize = 0;
		m_pStreamInfo->m_iSampleArrayIndex = 0;
		m_pStreamInfo->m_iAbortRequest = 0;
		m_pStreamInfo->m_iRefresh = 0;
		m_pStreamInfo->m_iShowMode = SHOW_MODE_NONE;
		m_pStreamInfo->m_pAudioRefreshThr = NULL;
		m_pStreamInfo->m_pVideoRefreshThr = NULL;
	}


	void CScreenAudioRecord::InitSdl()
	{
		//SDL初始化
		int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
		int  sdlinit = SDL_Init(flags);
		if (sdlinit)
		{
			char * sss = (char*)SDL_GetError();
			return;
		}

		//设置SDL事件状态
		SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
		SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
		SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

		//设置sdl显示模式
		m_pStreamInfo->m_iShowMode = SHOW_MODE_VIDEO;
	}

	DEVICE_INFO CScreenAudioRecord::GetDevice()
	{
		m_mapDeviceInfo.clear();
		//获取当前连接的视频设备
		int iRet = GetDeviceInfo(n_Video);
		if (iRet < 0){
			Write_Log(LOG_ERROR, "GetDevice : GetDeviceInfo error n_Video!");
			return m_mapDeviceInfo;
		}

		//获取当前连接输入的音频设备
		iRet = GetDeviceInfo(n_Audio);
		if (iRet < 0){
			Write_Log(LOG_ERROR, "GetDevice : GetDeviceInfo error n_Audio!");
			return m_mapDeviceInfo;
		}
		return m_mapDeviceInfo;
	}
	int CScreenAudioRecord::GetDeviceInfo(int _iDeviceType)
	{
 		int iRet = -1;
 		ICreateDevEnum *pDevEnum;
 		IEnumMoniker   *pEnumMon;
 		IMoniker	   *pMoniker;
 		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
 			IID_ICreateDevEnum, (LPVOID*)&pDevEnum);
 		if (SUCCEEDED(hr))
 		{
 			if (_iDeviceType == n_Video){
 				hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMon, 0);
 			}
 			else{
 				hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumMon, 0);
 			}
 			if (hr == S_FALSE)
 			{
 				TRACE("没有找到合适的音视频设备！\n");
 				hr = VFW_E_NOT_FOUND;
 				return hr;
 			}
 			pEnumMon->Reset();
 			ULONG cFetched;
 			while (hr = pEnumMon->Next(1, &pMoniker, &cFetched), hr == S_OK)
 			{
 				IPropertyBag *pProBag;
 				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (LPVOID*)&pProBag);
 				if (SUCCEEDED(hr))
 				{
 					VARIANT varTemp;
 					varTemp.vt = VT_BSTR;
 					hr = pProBag->Read(L"FriendlyName", &varTemp, NULL);
 					if (SUCCEEDED(hr))
 					{
 						iRet = 0;
 						_bstr_t bstr_t(varTemp.bstrVal);
 						std::string strDeviceName = bstr_t;
 
 						//将取到的设备名称存入容器中
 						std::vector<std::string> vecDeviceName;
 						vecDeviceName.clear();
 						vecDeviceName.push_back(strDeviceName);
 						SysFreeString(varTemp.bstrVal);
 
 						//如果容器相应的键值有值就量进行相应的添加处理，没值的话整体进行添加处理
 						std::map<int, std::vector<std::string>>::iterator iterInfo = m_mapDeviceInfo.find(_iDeviceType);
 						if (iterInfo == m_mapDeviceInfo.end()){
 							m_mapDeviceInfo.insert(map<int, std::vector<std::string>>::value_type(_iDeviceType, vecDeviceName));
 						}
 						else{
 							m_mapDeviceInfo[_iDeviceType].push_back(strDeviceName);
 						}
 					}
 					pProBag->Release();
 				}
 				pMoniker->Release();
 			}
 			pEnumMon->Release();
 		}
 
 		return iRet;
/*		return 0;*/
	}

	int CScreenAudioRecord::SetRecordInfo(const RECORD_INFO& recordInfo)
	{
		m_recordInfo = recordInfo;
		return 0;
	}

	int CScreenAudioRecord::CheckRecordInfo()
	{
		if (m_recordInfo.rtmp_url == "" || m_recordInfo.video_dst_height == 0
			|| m_recordInfo.video_dst_width == 0 || m_recordInfo.video_frame_rate == 0
			|| m_recordInfo.video_device_name == "" || m_recordInfo.audio_device_name == ""
			|| m_recordInfo.preview_hwnd == NULL){
			Write_Log(LOG_ERROR, "CheckRecordInfo : Paramer error!");
			return -1;
		}
		return 0;
	}

	int CScreenAudioRecord::StartRecord()
	{
		if (m_bPushStatus){
			return -1;
		}
		m_bPushStatus = TRUE;
		if (CheckRecordInfo() < 0){
			Write_Log(LOG_ERROR, "StartRecord : CheckRecordInfo error!");
			return -1;
		}

		if (CreateVideoWindow() < 0){
			Write_Log(LOG_ERROR, "StartRecord : CreateVideoWindow error!");
			return -1;
		}

		if (OpenCamera() < 0){
			Write_Log(LOG_ERROR, "StartRecord : OpenCamera error!");
			return -1;
		}

		if (OpenAduio() < 0){
			Write_Log(LOG_ERROR, "StartRecord : OpenAduio error!");
			return -1;
		}

		if (OpenRtmpAddr() < 0){
			Write_Log(LOG_ERROR, "StartRecord : OpenRtmpAddr error!");
			return -1;
		}

		//开启视频采集线程
		if (m_pStreamInfo->m_pVideoThr == NULL){
			m_pStreamInfo->m_pVideoThr = SDL_CreateThread(video_thr, NULL, (void*)this);
			if (m_pStreamInfo->m_pVideoThr == NULL){
				Write_Log(LOG_ERROR, "StartRecord : SDL_CreateThread(video_thr) error!");
				return -1;
			}
		}

		//开启音频采集线程
		if (m_pStreamInfo->m_pAudioThr == NULL){
			m_pStreamInfo->m_pAudioThr = SDL_CreateThread(audio_thr, NULL, (void*)this);
			if (m_pStreamInfo->m_pAudioThr == NULL){
				Write_Log(LOG_ERROR, "StartRecord : SDL_CreateThread(audio_thr) error!");
				return -1;
			}
		}

		//开启推送线程
		if (m_pWriteStreamThrid == NULL){
			m_pWriteStreamThrid = SDL_CreateThread(write_frame_thr, NULL, (void*)this);
			if (NULL == m_pWriteStreamThrid){
				Write_Log(LOG_ERROR, "StartRecord : SDL_CreateThread(write_frame_thr) error!");
				return -1;
			}
		}

		//开启采集线程
		if (m_pPushStreamThrid == NULL){
			m_pPushStreamThrid = SDL_CreateThread(push_thr, NULL, (void*)this);
			if (NULL == m_pPushStreamThrid){
				Write_Log(LOG_ERROR, "StartRecord : SDL_CreateThread(push_thr) error!");
				return -1;
			}
		}
		return 0;
	}

	int CScreenAudioRecord::StopRecord()
	{
		Write_Log(LOG_INFO, "StopRecord : Stop and delete paramer!");
		//释放相关资源
		m_blPushSuccess = FALSE;
		m_bPushStatus = FALSE;
		m_blCreateVideoWin = FALSE;

		if (NULL != m_pStreamInfo){
			m_pStreamInfo->m_iAbortRequest = 1;

			Write_Log(LOG_INFO, "StopRecord : Stop push stream!");
			if (NULL != m_pPushStreamThrid){
				SDL_WaitThread(m_pPushStreamThrid, NULL);
				m_pPushStreamThrid = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : Stop write stream!");
			if (NULL != m_pWriteStreamThrid){
				SDL_WaitThread(m_pWriteStreamThrid, NULL);
				m_pWriteStreamThrid = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : Stop video stream!");
			if (m_pStreamInfo->m_pVideoThr){
				SDL_WaitThread(m_pStreamInfo->m_pVideoThr, NULL);
				m_pStreamInfo->m_pVideoThr = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : Stop audio stream!");
			if (m_pStreamInfo->m_pAudioThr){
				SDL_WaitThread(m_pStreamInfo->m_pAudioThr, NULL);
				m_pStreamInfo->m_pAudioThr = NULL;
			}


			if (m_pStreamInfo->m_pVideoRefreshThr){
				SDL_WaitThread(m_pStreamInfo->m_pVideoRefreshThr, NULL);
				m_pStreamInfo->m_pVideoRefreshThr = NULL;
			}
			if (m_pStreamInfo->m_pAudioRefreshThr){
				SDL_WaitThread(m_pStreamInfo->m_pAudioRefreshThr, NULL);
				m_pStreamInfo->m_pAudioRefreshThr = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : sws_freeContext(m_pStreamInfo->m_pVideoSwsCtx);!");
			if (m_pStreamInfo->m_pVideoSwsCtx){
				sws_freeContext(m_pStreamInfo->m_pVideoSwsCtx);
				m_pStreamInfo->m_pVideoSwsCtx = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : av_audio_fifo_free(m_pStreamInfo->m_pAudioFifo);");
			if (m_pStreamInfo->m_pAudioFifo){
				av_audio_fifo_free(m_pStreamInfo->m_pAudioFifo);
				m_pStreamInfo->m_pAudioFifo = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : av_fifo_free(m_pStreamInfo->m_pVideoFifo);");
			if (m_pStreamInfo->m_pVideoFifo){
				av_fifo_free(m_pStreamInfo->m_pVideoFifo);
				m_pStreamInfo->m_pVideoFifo = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : SDL_DestroyMutex(m_pStreamInfo->m_pAudioMutex);");
			if (m_pStreamInfo->m_pAudioMutex){
				SDL_DestroyMutex(m_pStreamInfo->m_pAudioMutex);
				m_pStreamInfo->m_pAudioMutex = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : SDL_DestroyMutex(m_pStreamInfo->m_pVideoMutex);");
			if (m_pStreamInfo->m_pVideoMutex){
				SDL_DestroyMutex(m_pStreamInfo->m_pVideoMutex);
				m_pStreamInfo->m_pVideoMutex = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : av_freep(&m_pStreamInfo->m_pPushPicSize);");
			if (m_pStreamInfo->m_pPushPicSize){
				av_freep(&m_pStreamInfo->m_pPushPicSize);
			}
			Write_Log(LOG_INFO, "StopRecord : avcodec_close(m_pStreamInfo->m_pVideoStream->codec);");
			if (m_pStreamInfo->m_pVideoStream){
				if (m_pStreamInfo->m_pVideoStream->codec){
					avcodec_close(m_pStreamInfo->m_pVideoStream->codec);
					m_pStreamInfo->m_pVideoStream->codec = NULL;
				}
			}
			Write_Log(LOG_INFO, "StopRecord : avcodec_close(m_pStreamInfo->m_pAudioStream->codec);");
			if (m_pStreamInfo->m_pAudioStream){
				if (m_pStreamInfo->m_pAudioStream->codec){
					avcodec_close(m_pStreamInfo->m_pAudioStream->codec);
					m_pStreamInfo->m_pAudioStream->codec = NULL;
				}
			}
			Write_Log(LOG_INFO, "StopRecord : SDL_DestroyTexture(m_pStreamInfo->m_pSdlTexture);");
			if (m_pStreamInfo->m_pSdlTexture){
				SDL_DestroyTexture(m_pStreamInfo->m_pSdlTexture);
				m_pStreamInfo->m_pSdlTexture = NULL;
			}
			Write_Log(LOG_INFO, "StopRecord : SDL_DestroyRenderer(m_pStreamInfo->m_pSdlRender);");
			if (m_pStreamInfo->m_pSdlRender){
				SDL_DestroyRenderer(m_pStreamInfo->m_pSdlRender);
				m_pStreamInfo->m_pSdlRender = NULL;
			}
			if (NULL != m_pStreamInfo->m_pShowScreen){
				m_pStreamInfo->m_pShowScreen = NULL;
			}
			m_pStreamInfo->m_iAbortRequest = 0;
		}

		if (NULL != m_pCodecVideoCtx){
			avcodec_close(m_pCodecVideoCtx);
			m_pCodecVideoCtx = NULL;
		}

		if (NULL != m_pCodecAudioCtx){
			avcodec_close(m_pCodecAudioCtx);
			m_pCodecAudioCtx = NULL;
		}

		if (NULL != m_pFmtVideoCtx){
			avformat_close_input(&m_pFmtVideoCtx);
			m_pFmtVideoCtx = NULL;
		}

		if (NULL != m_pFmtAudioCtx){
			avformat_close_input(&m_pFmtAudioCtx);
			m_pFmtAudioCtx = NULL;
		}

		if (NULL != m_pFmtRtmpCtx){
			if (m_pFmtRtmpCtx && !(m_pFmtRtmpCtx->oformat->flags & AVFMT_NOFILE))
				avio_close(m_pFmtRtmpCtx->pb);
			m_pFmtRtmpCtx = NULL;
		}

		AutoLock l(&m_CritSec);
		list<AVPacket*>::iterator iter;
		AVPacket* pData = NULL;
		for (iter = m_listPacket.begin(); iter != m_listPacket.end(); iter++){
			pData = *iter;
			m_listPacket.erase(iter);
			av_free_packet(pData);
			pData = NULL;
			break;
		}

		return 0;
	}

	int CScreenAudioRecord::CreateVideoWindow()
	{
		if (m_blCreateVideoWin){
			return 0;
		}

		int iRet = -1;

		//将CSTATIC控件和sdl显示窗口关联 
		if (m_recordInfo.preview_hwnd != NULL){
			if (NULL != m_pStreamInfo && NULL == m_pStreamInfo->m_pShowScreen){
				m_pStreamInfo->m_pShowScreen = SDL_CreateWindowFrom((void*)m_recordInfo.preview_hwnd);
				if (m_pStreamInfo->m_pShowScreen == NULL){
					Write_Log(LOG_ERROR, "CreateVideoWindow : SDL_CreateWindowFrom error!");
					return iRet;
				}
			}
		}

		iRet = 0;
		return iRet;
	}

	int CScreenAudioRecord::OpenCamera()
	{
		Write_Log(LOG_INFO, "OpenCamera : OpenCamera.");
		int iRet = -1;
		AVInputFormat  *pVideoInputFmt = NULL;
		AVCodec		   *pCodec = NULL;

		if (NULL == m_pStreamInfo){
			Write_Log(LOG_ERROR, "OpenCamera : m_pStreamInfo is null!");
			return iRet;
		}

		pVideoInputFmt = av_find_input_format("dshow");
		if (pVideoInputFmt == NULL){
			Write_Log(LOG_ERROR, "OpenCamera : av_find_input_format error!");
			return iRet;
		}

		//打开摄像头
		m_pFmtVideoCtx = avformat_alloc_context();
		if (avformat_open_input(&m_pFmtVideoCtx, "video=screen-capture-recorder", pVideoInputFmt, NULL) != 0){
			Write_Log(LOG_INFO, "OpenCamera : video avformat_open_input error.");
			return iRet;
		}
		if (avformat_find_stream_info(m_pFmtVideoCtx, NULL) < 0){
			Write_Log(LOG_INFO, "OpenCamera : video avformat_find_stream_info error.");
			return iRet;
		}

		if (NULL == m_pFmtVideoCtx->streams){
			Write_Log(LOG_ERROR, "OpenCamera : m_pFmtVideoCtx->streams is null!");
			return iRet;
		}

		for (int i = 0; i < m_pFmtVideoCtx->nb_streams; i++){
			if (m_pFmtVideoCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
				m_iVideoIndex = i;
				break;
			}
		}

		if (m_iVideoIndex < 0){
			Write_Log(LOG_ERROR, "OpenCamera : m_iVideoIndex < 0!");
			return iRet;
		}
		m_pFmtVideoCtx->streams[m_iVideoIndex]->time_base.den = m_recordInfo.video_frame_rate;
		m_pFmtVideoCtx->streams[m_iVideoIndex]->time_base.num = 1;

		m_pCodecVideoCtx = m_pFmtVideoCtx->streams[m_iVideoIndex]->codec;
		if (NULL == m_pCodecVideoCtx){
			Write_Log(LOG_ERROR, "OpenCamera : m_pCodecVideoCtx is null!");
			return iRet;
		}

		//获取视频的宽与高
		m_iSrcVideoHeight = m_pCodecVideoCtx->height;
		m_iSrcVideoWidth = m_pCodecVideoCtx->width;

		InitVideoWindow();

		//需要将采集信息写文件，打开解码器
		pCodec = avcodec_find_decoder(m_pCodecVideoCtx->codec_id);
		if (pCodec == NULL){
			Write_Log(LOG_INFO, "OpenCamera : video avcodec_find_decoder error.");
			return iRet;
		}
		if (avcodec_open2(m_pCodecVideoCtx, pCodec, NULL) < 0){
			Write_Log(LOG_INFO, "OpenCamera : video avcodec_open2 error.");
			return iRet;
		}

		iRet = 0;
		Write_Log(LOG_INFO, "OpenCamera : OpenCamera success.");
		return iRet;
	}

	int CScreenAudioRecord::InitVideoWindow()
	{
		int iRet = -1;
		RECT rectDisPlay;

		if (m_blCreateVideoWin){
			goto END;
		}
		::GetWindowRect(m_recordInfo.preview_hwnd, &rectDisPlay);
		m_pStreamInfo->m_xLeft = rectDisPlay.left;
		m_pStreamInfo->m_yTop = rectDisPlay.top;
		m_pStreamInfo->m_width = m_recordInfo.video_dst_width;
		m_pStreamInfo->m_height = m_recordInfo.video_dst_height;
		m_iWindowWidth = rectDisPlay.right - rectDisPlay.left;
		m_iWindowHeight = rectDisPlay.bottom - rectDisPlay.top;


		if (NULL == m_pStreamInfo->m_pShowScreen){
			goto END;
		}

		m_pStreamInfo->m_pSdlRender = SDL_CreateRenderer(m_pStreamInfo->m_pShowScreen, -1, 0);
		if (NULL == m_pStreamInfo->m_pSdlRender){
			Write_Log(LOG_ERROR, "InitVideoWindow : SDL_CreateRenderer error!");
			goto END;
		}

		if (UpdateSdlInfo() < 0){
			Write_Log(LOG_ERROR, "InitVideoWindow : UpdateSdlInfo error!");
			goto END;
		}

		m_blCreateVideoWin = TRUE;

		iRet = 0;
	END:
		return iRet;
	}

	int CScreenAudioRecord::UpdateSdlInfo()
	{
		int iRet = -1;

		if (NULL == m_pStreamInfo->m_pSdlRender){
			Write_Log(LOG_ERROR, "UpdateSdlInfo : m_pStreamInfo->m_pSdlRender is null!");
			goto END;
		}

		//获取视频宽高之后创建纹理
		m_pStreamInfo->m_pSdlTexture = SDL_CreateTexture(m_pStreamInfo->m_pSdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_recordInfo.video_dst_width, m_recordInfo.video_dst_height);
		if (NULL == m_pStreamInfo->m_pSdlTexture){
			Write_Log(LOG_ERROR, "UpdateSdlInfo : SDL_CreateTexture is error!");
			goto END;
		}

		m_pStreamInfo->m_pVideoSwsCtx = sws_getContext(m_iSrcVideoWidth, m_iSrcVideoHeight, m_pCodecVideoCtx->pix_fmt,
			m_recordInfo.video_dst_width, m_recordInfo.video_dst_height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		if (NULL == m_pStreamInfo->m_pVideoSwsCtx){
			Write_Log(LOG_ERROR, "UpdateSdlInfo : m_pStreamInfo->m_pVideoSwsCtx is null!");
			goto END;
		}

		iRet = 0;

	END:
		return iRet;
	}

	int CScreenAudioRecord::OpenAduio()
	{
		Write_Log(LOG_INFO, "OpenAduio : OpenAduio start.");
		int iRet = -1;
		AVInputFormat  *pAudioInputFmt = NULL;
		AVCodec		  *pCodec = NULL;

		pAudioInputFmt = av_find_input_format("dshow");
		if (pAudioInputFmt == NULL){
			Write_Log(LOG_ERROR, "OpenAduio : pAudioInputFmt is null!");
			return iRet;
		}

		//打开麦克风
		m_pFmtAudioCtx = avformat_alloc_context();
		if (avformat_open_input(&m_pFmtAudioCtx, "audio=virtual-audio-capturer", pAudioInputFmt, NULL) != 0){
			Write_Log(LOG_INFO, "OpenAduio : audio avformat_open_input error.");
			return iRet;
		}
		if (avformat_find_stream_info(m_pFmtAudioCtx, NULL) < 0){
			Write_Log(LOG_INFO, "OpenAduio : audio avformat_find_stream_info error.");
			return iRet;
		}

		for (int i = 0; i < m_pFmtAudioCtx->nb_streams; i++){
			if (m_pFmtAudioCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
				m_iAudioIndex = i;
				break;
			}
		}

		if (m_iAudioIndex < 0){
			return iRet;
		}

		m_pCodecAudioCtx = m_pFmtAudioCtx->streams[m_iAudioIndex]->codec;
		if (NULL == m_pCodecAudioCtx){
			return iRet;
		}

		//需要将采集信息写文件，打开解码器
		pCodec = avcodec_find_decoder(m_pCodecAudioCtx->codec_id);
		if (pCodec == NULL){
			Write_Log(LOG_INFO, "OpenAduio : audio avcodec_find_decoder error.");
			return iRet;
		}
		if (avcodec_open2(m_pCodecAudioCtx, pCodec, NULL) < 0){
			Write_Log(LOG_INFO, "OpenAduio : audio avcodec_open2 error.");
			return iRet;
		}

		iRet = 0;
		Write_Log(LOG_INFO, "OpenAduio : OpenAduio success.");
		return iRet;
	}


	int CScreenAudioRecord::OpenRtmpAddr()
	{
		Write_Log(LOG_INFO, "OpenRtmpAddr : OpenRtmpAddr start.");
		int iRet = -1;
		AVOutputFormat		*pStreamOutfmt = NULL;

		if (NULL == m_pStreamInfo){
			return iRet;
		}

		//根据推流地址获取到AVFormatContext
		avformat_alloc_output_context2(&m_pFmtRtmpCtx, NULL, "flv", m_recordInfo.rtmp_url);
		if (NULL == m_pFmtRtmpCtx){
			Write_Log(LOG_ERROR, "OpenRtmpAddr : avformat_alloc_output_context2 is error!");
			return iRet;
		}

		pStreamOutfmt = m_pFmtRtmpCtx->oformat;
		if (NULL == pStreamOutfmt){
			return iRet;
		}

		if (NULL == m_pCodecVideoCtx){
			return iRet;
		}
		if (OpenRtmpUrl() < 0){
			Write_Log(LOG_ERROR, "OpenRtmpAddr : OpenRtmpUrl is error!");
			return iRet;
		}

		//写头
		if (!(pStreamOutfmt->flags & AVFMT_NOFILE)) {
			iRet = avio_open(&m_pFmtRtmpCtx->pb, m_recordInfo.rtmp_url, AVIO_FLAG_WRITE);
			if (iRet < 0) {
				Write_Log(LOG_ERROR, "OpenRtmpAddr : avio_open is error!");
				return iRet;
			}
		}

		iRet = avformat_write_header(m_pFmtRtmpCtx, NULL);
		if (iRet < 0) {
			Write_Log(LOG_ERROR, "OpenRtmpAddr : avformat_write_header is error!");
			return iRet;
		}
		Write_Log(LOG_INFO, "OpenRtmpAddr : OpenRtmpAddr success.");
		return iRet;
	}


	int CScreenAudioRecord::OpenRtmpUrl()
	{
		Write_Log(LOG_INFO, "OpenRtmpUrl : OpenRtmpUrl start.");
		int iRet = -1;
		//视频推流信息
		if (NULL != m_pFmtVideoCtx){
			m_pStreamInfo->m_pVideoStream = avformat_new_stream(m_pFmtRtmpCtx, NULL);
			if (!m_pStreamInfo->m_pVideoStream) {
				Write_Log(LOG_ERROR, "OpenRtmpUrl : avformat_new_stream video is error!");
				goto END;
			}
			m_pStreamInfo->m_pVideoStream->codec->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
			m_pStreamInfo->m_pVideoStream->codec->codec_tag = 0;
			m_pStreamInfo->m_pVideoStream->codec->height = m_recordInfo.video_dst_height;
			m_pStreamInfo->m_pVideoStream->codec->width = m_recordInfo.video_dst_width;
			m_pStreamInfo->m_pVideoStream->codec->time_base.den = m_recordInfo.video_frame_rate;
			m_pStreamInfo->m_pVideoStream->codec->time_base.num = 1;
			m_pStreamInfo->m_pVideoStream->codec->sample_aspect_ratio = m_pCodecVideoCtx->sample_aspect_ratio;
			m_pStreamInfo->m_pVideoStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
			// take first format from list of supported formats
			m_pStreamInfo->m_pVideoStream->codec->bit_rate = 900000;
			m_pStreamInfo->m_pVideoStream->codec->rc_max_rate = 900000;
			m_pStreamInfo->m_pVideoStream->codec->rc_min_rate = 900000;
			m_pStreamInfo->m_pVideoStream->codec->gop_size = m_pCodecVideoCtx->gop_size;
			m_pStreamInfo->m_pVideoStream->codec->qmin = 5;
			m_pStreamInfo->m_pVideoStream->codec->qmax = 51;
			m_pStreamInfo->m_pVideoStream->codec->max_b_frames = m_pCodecVideoCtx->max_b_frames;
			m_pStreamInfo->m_pVideoStream->r_frame_rate = m_pFmtVideoCtx->streams[m_iVideoIndex]->r_frame_rate;

			m_iVideoOutIndex = m_pStreamInfo->m_pVideoStream->index;

			if (m_pFmtRtmpCtx->oformat->flags & AVFMT_GLOBALHEADER)
				m_pStreamInfo->m_pVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

			//打开视频编码器
			if ((avcodec_open2(m_pStreamInfo->m_pVideoStream->codec, m_pStreamInfo->m_pVideoStream->codec->codec, NULL)) < 0){
				Write_Log(LOG_ERROR, "OpenRtmpUrl : avcodec_open2 video is error!");
				goto END;
			}

			m_pStreamInfo->m_pVideoFifo = av_fifo_alloc(30 * avpicture_get_size(AV_PIX_FMT_YUV420P, m_pStreamInfo->m_pVideoStream->codec->width, m_pStreamInfo->m_pVideoStream->codec->height));
		}

		//音频推流信息
		if (NULL != m_pFmtAudioCtx){
			m_pStreamInfo->m_pAudioStream = avformat_new_stream(m_pFmtRtmpCtx, NULL);
			if (NULL == m_pStreamInfo->m_pAudioStream){
				Write_Log(LOG_ERROR, "OpenRtmpUrl : avformat_new_stream audio is error!");
				goto END;
			}
			if (NULL == m_pFmtRtmpCtx->streams[m_iAudioIndex]){
				goto END;
			}
			m_pStreamInfo->m_pAudioStream->codec->codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
			m_pStreamInfo->m_pAudioStream->codec->sample_rate = m_pCodecAudioCtx->sample_rate;
			m_pStreamInfo->m_pAudioStream->codec->channel_layout = m_pFmtRtmpCtx->streams[m_iAudioIndex]->codec->channel_layout;
			m_pStreamInfo->m_pAudioStream->codec->channels = av_get_channel_layout_nb_channels(m_pStreamInfo->m_pAudioStream->codec->channel_layout);
			if (m_pStreamInfo->m_pAudioStream->codec->channel_layout == 0){
				m_pStreamInfo->m_pAudioStream->codec->channel_layout = AV_CH_LAYOUT_STEREO;
				m_pStreamInfo->m_pAudioStream->codec->channels = av_get_channel_layout_nb_channels(m_pStreamInfo->m_pAudioStream->codec->channel_layout);
			}
			m_pStreamInfo->m_pAudioStream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
			m_pStreamInfo->m_pAudioStream->codec->sample_fmt = m_pStreamInfo->m_pAudioStream->codec->codec->sample_fmts[0];
			AVRational time_base = { 1, m_pStreamInfo->m_pAudioStream->codec->sample_rate };
			m_pStreamInfo->m_pAudioStream->time_base = time_base;
			m_pFmtAudioCtx->streams[0]->time_base = time_base;
			m_iAudioOutIndex = m_pStreamInfo->m_pAudioStream->index;

			m_pStreamInfo->m_pAudioStream->codec->codec_tag = 0;
			if (m_pFmtRtmpCtx->oformat->flags & AVFMT_GLOBALHEADER)
				m_pStreamInfo->m_pAudioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

			//打开音频编码器
			if (avcodec_open2(m_pStreamInfo->m_pAudioStream->codec, m_pStreamInfo->m_pAudioStream->codec->codec, 0) < 0){
				Write_Log(LOG_ERROR, "OpenRtmpUrl : avcodec_open2 audio is error!");
				goto END;
			}
		}

		iRet = 0;
		Write_Log(LOG_INFO, "OpenRtmpUrl : OpenRtmpUrl success.");
	END:
		return iRet;
	}

	CString CScreenAudioRecord::GetWorkDir()
	{
		HMODULE module = GetModuleHandle(0);
		wchar_t pFileName[MAX_PATH];
		GetModuleFileName(module, pFileName, MAX_PATH);

		CString csFullPath(pFileName);
		int nPos = csFullPath.ReverseFind(_T('\\'));
		if (nPos < 0)
			return CString("");
		else
			return csFullPath.Left(nPos);
	}

	int video_thr(LPVOID lpParam)
	{
		Write_Log(LOG_INFO, "video_thr : video_thr start.");
		int iRet = -1;
		struct_stream_info	*	strct_streaminfo = NULL;
		int						iVideoPic = 0;
		int64_t					start_time = 0;
		AVPacket		pkt;

		CScreenAudioRecord* pThis = (CScreenAudioRecord*)lpParam;
		if (pThis == NULL){
			return iRet;
		}
		if (NULL == pThis->m_pFmtVideoCtx || NULL == pThis->m_pCodecVideoCtx){
			return iRet;
		}

		strct_streaminfo = pThis->m_pStreamInfo;
		if (NULL == strct_streaminfo){
			return iRet;
		}

		AVFrame	*pFrame;
		pFrame = avcodec_alloc_frame();
		AVFrame *picture = avcodec_alloc_frame();
		//AVFrame *SDL_picture = avcodec_alloc_frame();

		int size = avpicture_get_size(pThis->m_pCodecVideoCtx->pix_fmt, pThis->m_recordInfo.video_dst_width, pThis->m_recordInfo.video_dst_width);

		//申请picture
		if (avpicture_alloc((AVPicture*)picture, AV_PIX_FMT_YUV420P, pThis->m_iSrcVideoWidth, pThis->m_iSrcVideoHeight) < 0){
			Write_Log(LOG_ERROR, "video_thr : avpicture_alloc is error!");
			return iRet;
		}

		int height = pThis->m_recordInfo.video_dst_height;
		int width = pThis->m_recordInfo.video_dst_width;
		int y_size = height*width;

		//原视频区域
		SDL_Rect sdlSrcRect;
		sdlSrcRect.x = 0;
		sdlSrcRect.y = 0;
		sdlSrcRect.w = pThis->m_iSrcVideoWidth;
		sdlSrcRect.h = pThis->m_iSrcVideoHeight;

		//从摄像头获取数据
		while (1){
			if (strct_streaminfo->m_iAbortRequest){
				break;
			}
			pkt.data = NULL;
			pkt.size = 0;
			if (av_read_frame(pThis->m_pFmtVideoCtx, &pkt) >= 0){
				//解码显示
				if (pkt.stream_index != 0){
					av_free_packet(&pkt);
					continue;
				}
				iRet = avcodec_decode_video2(pThis->m_pCodecVideoCtx, pFrame, &iVideoPic, &pkt);
				if (iRet < 0){
					Write_Log(LOG_ERROR, "video_thr : avcodec_decode_video2 is error!");
					goto END;
				}
				av_free_packet(&pkt);
				if (iVideoPic <= 0){
					goto END;
				}

				if (sws_scale(strct_streaminfo->m_pVideoSwsCtx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pThis->m_iSrcVideoHeight,
					picture->data, picture->linesize) < 0){
					Write_Log(LOG_ERROR, "video_thr : sws_scale is error!");
					goto END;
				}

				//显示视频的区域
				SDL_Rect sdlRect;
				sdlRect.x = 0;
				sdlRect.y = 0;
				sdlRect.w = pThis->m_iWindowWidth;
				sdlRect.h = pThis->m_iWindowHeight;
				//显示视频
				SDL_UpdateYUVTexture(strct_streaminfo->m_pSdlTexture, NULL,
					picture->data[0], picture->linesize[0],
					picture->data[1], picture->linesize[1],
					picture->data[2], picture->linesize[2]);
				SDL_RenderClear(strct_streaminfo->m_pSdlRender);
				SDL_RenderCopy(strct_streaminfo->m_pSdlRender, strct_streaminfo->m_pSdlTexture, &sdlSrcRect, &sdlRect);
				SDL_RenderPresent(strct_streaminfo->m_pSdlRender);

				int iVideoFifoSize = av_fifo_space(strct_streaminfo->m_pVideoFifo);
				if (iVideoFifoSize >= size){
					SDL_mutexP(strct_streaminfo->m_pVideoMutex);
					av_fifo_generic_write(strct_streaminfo->m_pVideoFifo, picture->data[0], y_size, NULL);
					av_fifo_generic_write(strct_streaminfo->m_pVideoFifo, picture->data[1], y_size / 4, NULL);
					av_fifo_generic_write(strct_streaminfo->m_pVideoFifo, picture->data[2], y_size / 4, NULL);
					SDL_mutexV(strct_streaminfo->m_pVideoMutex);
				}
			}
		}

		iRet = 1;
	END:
		av_frame_free(&pFrame);
		av_frame_free(&picture);
		Write_Log(LOG_INFO, "video_thr : video capture is over.");
		return iRet;
	}

	int audio_thr(LPVOID lpParam)
	{
		Write_Log(LOG_INFO, "audio_thr : audio_thr start.");
		//采集音频，进行储存
		int iRet = -1;
		AVPacket		pkt;
		AVFrame*		pFrame = NULL;

		CScreenAudioRecord* pThis = (CScreenAudioRecord*)lpParam;
		if (pThis == NULL){
			return iRet;
		}
		if (NULL == pThis->m_pFmtAudioCtx){
			return iRet;
		}
		AVCodecContext* pCodec = pThis->m_pFmtAudioCtx->streams[0]->codec;
		if (NULL == pCodec){
			return iRet;
		}

		struct_stream_info* pStrctStreamInfo = pThis->m_pStreamInfo;
		if (NULL == pStrctStreamInfo){
			return iRet;
		}

		while (1){
			if (pStrctStreamInfo->m_iAbortRequest){
				break;
			}
			pkt.data = NULL;
			pkt.size = 0;
			if (av_read_frame(pThis->m_pFmtAudioCtx, &pkt) < 0){
				Write_Log(LOG_ERROR, "audio_thr : av_read_frame error!");
				continue;
			}

			if (!pFrame) {
				if (!(pFrame = avcodec_alloc_frame())){
					Write_Log(LOG_ERROR, "audio_thr : avcodec_alloc_frame error!");
					goto END;
				}
			}
			else{
				avcodec_get_frame_defaults(pFrame);
			}

			int gotframe = -1;
			if (avcodec_decode_audio4(pThis->m_pFmtAudioCtx->streams[0]->codec, pFrame, &gotframe, &pkt) < 0){
				Write_Log(LOG_ERROR, "audio_thr : avcodec_decode_audio4 error!");
				break;
			}
			av_free_packet(&pkt);

			if (!gotframe){
				//没有获取到数据，继续下一次
				continue;
			}

			SDL_mutexP(pStrctStreamInfo->m_pAudioMutex);
			if (NULL == pStrctStreamInfo->m_pAudioFifo){
				pStrctStreamInfo->m_pAudioFifo = av_audio_fifo_alloc(pThis->m_pFmtAudioCtx->streams[0]->codec->sample_fmt,
					pThis->m_pFmtAudioCtx->streams[0]->codec->channels, 30 * pFrame->nb_samples);
			}

			int buf_space = av_audio_fifo_space(pStrctStreamInfo->m_pAudioFifo);
			if (buf_space >= pFrame->nb_samples){
				av_audio_fifo_write(pStrctStreamInfo->m_pAudioFifo, (void **)pFrame->data, pFrame->nb_samples);
			}
			else if (buf_space > 0){
				av_audio_fifo_write(pStrctStreamInfo->m_pAudioFifo, (void **)pFrame->data, buf_space);
			}
			SDL_mutexV(pStrctStreamInfo->m_pAudioMutex);
		}
		iRet = 1;

	END:
		av_frame_free(&pFrame);
		Write_Log(LOG_INFO, "audio_thr : audio capture is over.");
		return iRet;
	}

	int push_thr(LPVOID lpParam)
	{
		Write_Log(LOG_INFO, "push_thr : push_thr start.");
		int iRet = -1;
		int64_t cur_pts_v = 0, cur_pts_a = 0;
		int	frame_video_index = 0;
		int frame_audio_index = 0;

		CScreenAudioRecord *pThis = (CScreenAudioRecord*)lpParam;
		if (NULL == pThis){
			return iRet;
		}

		struct_stream_info* pStrctStreamInfo = pThis->m_pStreamInfo;
		if (NULL == pStrctStreamInfo){
			return iRet;
		}

		if (NULL == pThis->m_pFmtRtmpCtx){
			return iRet;
		}

		AVFrame *picture = av_frame_alloc();
		int size = avpicture_get_size(pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoIndex]->codec->pix_fmt,
			pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoIndex]->codec->width, pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoIndex]->codec->height);
		pStrctStreamInfo->m_pPushPicSize = (uint8_t*)av_malloc(size);

		AVFrame *frame;//声音帧
		while (1){
			if (pStrctStreamInfo->m_iAbortRequest){
				break;
			}
			if (av_compare_ts(cur_pts_v, pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->time_base,
				cur_pts_a, pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->time_base) <= 0){
				//视频处理
				if (av_fifo_size(pStrctStreamInfo->m_pVideoFifo) >= size){
					SDL_mutexP(pStrctStreamInfo->m_pVideoMutex);
					av_fifo_generic_read(pStrctStreamInfo->m_pVideoFifo, pStrctStreamInfo->m_pPushPicSize, size, NULL);
					SDL_mutexV(pStrctStreamInfo->m_pVideoMutex);

					avpicture_fill((AVPicture *)picture, pStrctStreamInfo->m_pPushPicSize,
						pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->codec->pix_fmt,
						pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->codec->width,
						pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->codec->height);

					picture->pts = frame_video_index * ((pThis->m_pFmtVideoCtx->streams[0]->time_base.den / pThis->m_pFmtVideoCtx->streams[0]->time_base.num) / pThis->m_recordInfo.video_frame_rate);

					int got_picture = 0;
					AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));//视频包
					av_init_packet(pkt);
					pkt->data = NULL;
					pkt->size = 0;
					iRet = avcodec_encode_video2(pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->codec, pkt, picture, &got_picture);
					if (iRet < 0){
						//编码错误,不理会此帧
						av_free_packet(pkt);
						continue;
					}

					if (got_picture == 1){
						pkt->stream_index = pThis->m_iVideoOutIndex;
						pkt->pts = av_rescale_q(pkt->pts, pThis->m_pFmtVideoCtx->streams[0]->time_base,
							pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->time_base);
						pkt->dts = av_rescale_q(pkt->dts, pThis->m_pFmtVideoCtx->streams[0]->time_base,
							pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->time_base);
						if (pThis->m_pFmtRtmpCtx->streams[pThis->m_iVideoOutIndex]->codec->coded_frame->key_frame){
							pkt->flags |= AV_PKT_FLAG_KEY;
						}

						cur_pts_v = pkt->pts;
						set_packet(pkt);
						pThis->m_blPushSuccess = TRUE;
						frame_video_index++;
					}
				}
			}
			else{
				//Audio
				if (NULL == pStrctStreamInfo->m_pAudioFifo){
					continue;//还未初始化fifo
				}
				if (av_audio_fifo_size(pStrctStreamInfo->m_pAudioFifo) >=
					(pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size > 0 ? pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size : 1024))
				{
					frame = av_frame_alloc();
					frame->nb_samples = pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size > 0 ? pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size : 1024;
					frame->channel_layout = pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->channel_layout;
					frame->format = pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->sample_fmt;
					frame->sample_rate = pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->sample_rate;
					av_frame_get_buffer(frame, 0);

					SDL_mutexP(pStrctStreamInfo->m_pAudioMutex);
					av_audio_fifo_read(pStrctStreamInfo->m_pAudioFifo, (void **)frame->data,
						(pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size > 0 ? pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size : 1024));
					SDL_mutexV(pStrctStreamInfo->m_pAudioMutex);

					AVPacket *pkt_out = (AVPacket *)av_malloc(sizeof(AVPacket));;
					av_init_packet(pkt_out);
					int got_picture = -1;
					pkt_out->data = NULL;
					pkt_out->size = 0;

					frame->pts = frame_audio_index * pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec->frame_size;
					if (avcodec_encode_audio2(pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->codec, pkt_out, frame, &got_picture) < 0){
						av_free_packet(pkt_out);
						continue;
					}
					if (NULL == pkt_out->data){
						av_free_packet(pkt_out);
						continue;
					}
					if (got_picture == 1){
						pkt_out->stream_index = pThis->m_iAudioOutIndex;

						pkt_out->pts = av_rescale_q(pkt_out->pts, pThis->m_pFmtAudioCtx->streams[0]->time_base,
							pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->time_base);
						pkt_out->dts = av_rescale_q(pkt_out->dts, pThis->m_pFmtAudioCtx->streams[0]->time_base,
							pThis->m_pFmtRtmpCtx->streams[pThis->m_iAudioOutIndex]->time_base);

						cur_pts_a = pkt_out->pts;
						set_packet(pkt_out);
						pThis->m_blPushSuccess = TRUE;
						frame_audio_index++;
					}
					av_frame_free(&frame);
				}
			}
		}

		//推送结束
		Write_Log(LOG_INFO, "push_thr : push stream is over.");
		av_write_trailer(pThis->m_pFmtRtmpCtx);
		av_frame_free(&picture);
		return iRet;
	}

	AVPacket* get_packet()
	{
		AutoLock l(&m_CritSec);
		if (m_listPacket.empty()){
			return NULL;
		}

		list<AVPacket*>::iterator iter;
		AVPacket* pData = NULL;
		for (iter = m_listPacket.begin(); iter != m_listPacket.end(); iter++)
		{
			pData = *iter;
			m_listPacket.erase(iter);
			break;
		}
		return pData;
	}

	void set_packet(AVPacket* pkt)
	{
		AutoLock l(&m_CritSec);
		m_listPacket.push_back(pkt);
	}

	int write_frame_thr(LPVOID lpParam)
	{
		int iRet = -1;
		CScreenAudioRecord* pThis = (CScreenAudioRecord*)lpParam;
		if (pThis == NULL){
			return iRet;
		}
		if (NULL == pThis->m_pFmtAudioCtx){
			return iRet;
		}
		AVCodecContext* pCodec = pThis->m_pFmtAudioCtx->streams[0]->codec;
		if (NULL == pCodec){
			return iRet;
		}

		struct_stream_info* pStrctStreamInfo = pThis->m_pStreamInfo;
		if (NULL == pStrctStreamInfo){
			return iRet;
		}
		while (true)
		{
			if (pStrctStreamInfo->m_iAbortRequest){
				break;
			}
			AVPacket* pPkt = get_packet();
			if (pPkt){
				if (pPkt->data != NULL){
					iRet = av_interleaved_write_frame(pThis->m_pFmtRtmpCtx, pPkt);
				}
				av_free_packet(pPkt);
				pPkt = NULL;
			}
		}
		Write_Log(LOG_INFO, "write_frame_thr : push over.");
		return iRet;
	}

	IScreenAudioClass* CreateScreenAudioRecorder()
	{
		IScreenAudioClass* pRecorder = new CScreenAudioRecord();
		return pRecorder;
	}

	void DestroyScreenAudioRecorder(IScreenAudioClass* pRecorder)
	{
		if (pRecorder)
			delete pRecorder;
	}

}