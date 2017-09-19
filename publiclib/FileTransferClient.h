//huangyubin @ 2016.2
//file transfer

#ifndef __FILE_TRANSFER_CLIENT__
#define __FILE_TRANSFER_CLIENT__
#include <afxwin.h>


#define WM_FILETRANSFER WM_USER+1896
enum FileTransferType
{
	FILE_ID = 1,
	FILE_SENDED_LEN,
	FILE_OPEN_FAIL,
	FILE_TOO_BIG,
	FILE_CONNECT_FAIL,
	FILE_SEND_ERROR,
	FILE_SEND_OVER
};



extern "C" __declspec(dllimport) class FileTransferEvent
{
public:
	virtual void On_FileTransferEvent(int nMsgType, int nMsgPramter) = 0;
};

extern "C" __declspec(dllimport) bool FileTransfer_UploadFile(const wchar_t* cszFileName, HWND hNotify, FileTransferEvent* pEvent = NULL);

#endif