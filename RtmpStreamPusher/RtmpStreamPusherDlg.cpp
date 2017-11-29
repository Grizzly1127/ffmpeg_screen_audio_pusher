
// RtmpStreamPusherDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RtmpStreamPusher.h"
#include "RtmpStreamPusherDlg.h"
#include "afxdialogex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRtmpStreamPusherDlg 对话框



CRtmpStreamPusherDlg::CRtmpStreamPusherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRtmpStreamPusherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRtmpStreamPusherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_START, m_btStart);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btStop);
	DDX_Control(pDX, IDC_EDIT_FRAMERATE, m_editFrameRate);
	DDX_Control(pDX, IDC_EDIT_URL, m_editUrl);
	DDX_Control(pDX, IDC_COMBO_VIDEO, m_cboDeviceVideo);
	DDX_Control(pDX, IDC_COMBO_AUDIO, m_cboDeviceAudio);
}

BEGIN_MESSAGE_MAP(CRtmpStreamPusherDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CRtmpStreamPusherDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CRtmpStreamPusherDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CRtmpStreamPusherDlg 消息处理程序

BOOL CRtmpStreamPusherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	m_editFrameRate.SetWindowTextW(_T("25"));
	m_editUrl.SetWindowTextW(_T("rtmp://video-center.alivecdn1.com/55667788/LiveStream?vhost=rtmp01.talk915.com"));

	m_pPusher = MR_CreateScreenAudioRecorder();


	m_mapDeviceInfo = MR_GetDevice(m_pPusher);

	m_cboDeviceVideo.ResetContent();
	m_cboDeviceAudio.ResetContent();
	//将获取到的设备信息插入下拉框中
	MediaType::DEVICE_INFO::iterator iter = m_mapDeviceInfo.begin();
	for (; iter != m_mapDeviceInfo.end(); iter++){
		if (MediaType::n_Video == iter->first){
			//视频设备
			for (int i = 0; i < iter->second.size(); i++){
				int iCount = m_cboDeviceVideo.GetCount();
				CharCode szDevice(iter->second[i].c_str());
				m_cboDeviceVideo.InsertString(iCount, szDevice.GetStringW());
			}
			m_cboDeviceVideo.SetCurSel(0);
		}
		else if (MediaType::n_Audio == iter->first){
			//音频设备
			for (int i = 0; i < iter->second.size(); i++){
				int iCount = m_cboDeviceAudio.GetCount();
				CharCode szDevice(iter->second[i].c_str());
				m_cboDeviceAudio.InsertString(iCount, szDevice.GetStringW());
			}
			m_cboDeviceAudio.SetCurSel(0);
		}
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRtmpStreamPusherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRtmpStreamPusherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRtmpStreamPusherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRtmpStreamPusherDlg::OnBnClickedButtonStart()
{
	// TODO:  在此添加控件通知处理程序代码
	int   cx = GetSystemMetrics(SM_CXSCREEN);
	int   cy = GetSystemMetrics(SM_CYSCREEN);
	CString cstrVideoDevName = _T("");
	CString cstrAudioDevName = _T("");
	m_cboDeviceVideo.GetWindowText(cstrVideoDevName);
	m_cboDeviceAudio.GetWindowText(cstrAudioDevName);
	if (cstrVideoDevName.IsEmpty()){
		cstrVideoDevName = _T("screen-capture-recorder");
	}
	if (cstrAudioDevName.IsEmpty()){
		cstrAudioDevName = _T("virtual-audio-capturer");
	}
	cstrVideoDevName = _T("video=") + cstrVideoDevName;
	cstrAudioDevName = _T("audio=") + cstrAudioDevName;

	CString strFrame = _T("");
	CString strUrl = _T("");
	m_editFrameRate.GetWindowTextW(strFrame);
	m_editUrl.GetWindowTextW(strUrl);
	CharCode szUrl(strUrl.GetBuffer(0));
	CharCode szVideoDev(cstrVideoDevName.GetBuffer(0));
	CharCode szAudioDev(cstrAudioDevName.GetBuffer(0));

	MediaType::RECORD_INFO record_info;
	strcpy_s(record_info.rtmp_url, szUrl.GetStringA());
	strcpy_s(record_info.video_device_name, szVideoDev.GetStringA());
	strcpy_s(record_info.audio_device_name, szAudioDev.GetStringA());
	record_info.video_dst_width = cx;
	record_info.video_dst_height = cy;
	record_info.video_frame_rate = _ttoi(strFrame);
	record_info.preview_hwnd = GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd();

	int ret = MR_SetRecordInfo(m_pPusher, record_info);
	ret = MR_StartRecord(m_pPusher);
}


void CRtmpStreamPusherDlg::OnBnClickedButtonStop()
{
	// TODO:  在此添加控件通知处理程序代码
	MR_StopRecord(m_pPusher);
}
