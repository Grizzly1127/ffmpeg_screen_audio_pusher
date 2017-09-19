#include "stdafx.h"
#include "MusicPlay.h"
#include "LogEx.h"

CMusicPlay::CMusicPlay()
: m_nStatus(PLAYER_STATUS::MUSIC_STOP)
, m_strCurMusicPath(_T(""))
, m_wDeviceID(NULL)
, mciError(0)
, m_dwTotalTime(0)
, m_dwSeekTo(0)
, m_nInterval(200)
, m_nOldTick(0)
, m_dwCurPos(0)
, m_bStopThread(TRUE)
{
}

CMusicPlay::~CMusicPlay()
{
	m_bStopThread = TRUE;
	WaitForStop();
}

void CMusicPlay::setNotifyHwnd(HWND hNotifyWindow, IMusicPlayEventHandler* pIMusicPlay)
{
	m_hNotifyWindow = hNotifyWindow;
	m_pIMusicPlay = pIMusicPlay;
}

void CMusicPlay::OpenFile(CString strFile)
{
	m_strCurMusicPath = strFile;

	MCI_OPEN_PARMS mciOpen;
	mciOpen.lpstrDeviceType = _T("mpegvideo");
	mciOpen.lpstrElementName = m_strCurMusicPath;
	Write_LogW(LOG_INFO, _T("CMusicPlay: OpenFile path = %s, mciOpen.lpstrDeviceType = %s"), m_strCurMusicPath, mciOpen.lpstrDeviceType);
	mciError = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);
	if (0 != mciError)
	{
		Write_Log(LOG_ERROR, "CMusicPlay: OpenFile path mciError = %d", mciError);
		m_wDeviceID = NULL;
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
	m_wDeviceID = mciOpen.wDeviceID;

	//获取总时长
	ThreadGetTotalTime();
	//关闭文件
	ThreadCloseFile();
}

void CMusicPlay::Play()
{
	AutoLock l(&m_CritSec);
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_PLAY);
	if (m_bStopThread)
	{
		m_bStopThread = FALSE;
		StartThread();
	}
}

void CMusicPlay::Pause()
{
	AutoLock l(&m_CritSec);
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_PAUSE);
}

void CMusicPlay::Resume()
{
	AutoLock l(&m_CritSec);
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_RESUME);
}

void CMusicPlay::Stop()
{
	AutoLock l(&m_CritSec);
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_STOP);
	return;
}

int CMusicPlay::GetStatus()
{
	return m_nStatus;
}

CString CMusicPlay::GetMusicPath()
{
	return m_strCurMusicPath;
}

DWORD CMusicPlay::GetTotalTime()
{
	return m_dwTotalTime;
}

void CMusicPlay::SetTimePostionInterval(int nInterval)
{
	m_nInterval = nInterval;
}

DWORD CMusicPlay::GetTimePostion()
{
	return m_dwCurPos;
}

void CMusicPlay::CloseFile()
{
	AutoLock l(&m_CritSec);
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_CLOSE);
}

void CMusicPlay::PlayFrom(DWORD dwTime)
{
	AutoLock l(&m_CritSec);
	m_dwSeekTo = dwTime;
	m_listTaskPool.push_back(PLAYER_STATUS::MUSIC_SEEK);
}

void CMusicPlay::ThreadProcMain(void)
{
	MCI_OPEN_PARMS mciOpen;
	mciOpen.lpstrDeviceType = _T("mpegvideo");
	mciOpen.lpstrElementName = m_strCurMusicPath;
	mciError = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);
	if (0 != mciError)
	{
		m_wDeviceID = NULL;
		return;
	}
	m_wDeviceID = mciOpen.wDeviceID;

	//获取总时长
	//ThreadGetTotalTime();
	while (true)
	{
		int nTaskId = 0;
		LIST_TASK::iterator iter = m_listTaskPool.begin();
		for (; iter != m_listTaskPool.end(); ++iter)
		{
			nTaskId = *iter;
			switch (nTaskId)
			{
			case PLAYER_STATUS::MUSIC_PLAY:ThreadPaly();break;
			case PLAYER_STATUS::MUSIC_PAUSE:ThreadPause();break;
			case PLAYER_STATUS::MUSIC_STOP:ThreadStop();break;
			case PLAYER_STATUS::MUSIC_RESUME:ThreadResume();break;
			case PLAYER_STATUS::MUSIC_SEEK:ThreadSeek();break;
			case PLAYER_STATUS::MUSIC_CLOSE:ThreadCloseFile();return;
			default:
				break;
			}
			m_listTaskPool.erase(iter);
			break;
		}
		if (m_nStatus == PLAYER_STATUS::MUSIC_PLAY)
		{
			int nNewTick = GetTickCount();
			if ((nNewTick - m_nOldTick) >= m_nInterval)
			{
				m_dwCurPos = ThreadGetTimePostion();
				if (m_pIMusicPlay)
					m_pIMusicPlay->OnTimePostion(m_dwCurPos);

				m_nOldTick = nNewTick;
			}
		}
		Sleep(200);
		if (m_bStopThread)
		{
			ThreadCloseFile();
			break;
		}

	}
}

void CMusicPlay::ThreadPaly()
{
	MCI_PLAY_PARMS mciPlay;
	if (m_hNotifyWindow)
		mciPlay.dwCallback = (DWORD)m_hNotifyWindow;
	mciError = mciSendCommand(m_wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)&mciPlay);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
	m_nStatus = PLAYER_STATUS::MUSIC_PLAY;
}


void CMusicPlay::ThreadPause()
{
	//如果已经是stop状态，就不要响应暂停了，否则上层获取的状态会错误
	//[bug:a音乐播放完，切换到b音乐播放，点击b音乐暂停，切换回a音乐，a音乐播放不了]
	if (PLAYER_STATUS::MUSIC_STOP == m_nStatus)
		return;

	mciError = mciSendCommand(m_wDeviceID, MCI_PAUSE, NULL, NULL);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
	m_nStatus = PLAYER_STATUS::MUSIC_PAUSE;
}

void CMusicPlay::ThreadStop()
{
	mciError = mciSendCommand(m_wDeviceID, MCI_STOP, NULL, NULL);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
	mciError = mciSendCommand(m_wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}

	m_dwCurPos = 0;
	m_nStatus = PLAYER_STATUS::MUSIC_STOP;
}

void CMusicPlay::ThreadResume()
{
	mciError = mciSendCommand(m_wDeviceID, MCI_RESUME, NULL, NULL);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
	m_nStatus = PLAYER_STATUS::MUSIC_PLAY;
}

void CMusicPlay::ThreadSeek()
{
	MCI_PLAY_PARMS PlayParms;
	PlayParms.dwFrom = m_dwSeekTo;
	mciError = mciSendCommand(m_wDeviceID, MCI_PLAY, MCI_FROM, (DWORD)(LPVOID)&PlayParms);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
}

void CMusicPlay::ThreadGetTotalTime()
{
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_LENGTH;
	mciError = mciSendCommand(m_wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&mciStatusParms);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		Write_Log(LOG_ERROR, "CMusicPlay : Get audio total time is error.");
		return;
	}
	m_dwTotalTime = mciStatusParms.dwReturn;
	Write_Log(LOG_INFO, "CMusicPlay : Audio total time is %d", m_dwTotalTime);
}

void CMusicPlay::ThreadCloseFile()
{
	mciError = mciSendCommand(m_wDeviceID, MCI_CLOSE, 0, NULL);
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return;
	}
}

DWORD CMusicPlay::ThreadGetTimePostion()
{
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_POSITION;
	mciError = mciSendCommand(m_wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID)&mciStatusParms);//关键,取得位置
	if (0 != mciError)
	{
		ErrorCode(mciError, __FUNCTION__);
		return 0;
	}
	return mciStatusParms.dwReturn;
}

void CMusicPlay::ErrorCode(int nErrCode, LPCSTR func)
{
	wchar_t buf[256];
	mciGetErrorString(mciError, buf, 256);
	if (m_pIMusicPlay)
		m_pIMusicPlay->OnGetErrorCode(mciError, buf, func);

}