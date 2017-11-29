
// RtmpStreamPusherDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "../publiclib/publicfun.h"
#include "../ffmpeg_screen_audio_pusher/IScreenAudioPusher.h"
#pragma comment(lib, "../Debug/publiclib.lib")
#pragma comment(lib, "../Debug/ffmpeg_screen_audio_pusher.lib")


// CRtmpStreamPusherDlg 对话框
class CRtmpStreamPusherDlg : public CDialog
{
// 构造
public:
	CRtmpStreamPusherDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_RTMPSTREAMPUSHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	CButton m_btStart;
	CButton m_btStop;
	CEdit m_editFrameRate;
	CEdit m_editUrl;
	void* m_pPusher;
	CComboBox m_cboDeviceVideo;
	CComboBox m_cboDeviceAudio;
	MediaType::DEVICE_INFO m_mapDeviceInfo;
};
