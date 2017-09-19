
#ifndef __SREENSHOT_H__
#define __SREENSHOT_H__
#include <afxwin.h>

#define SCREENSHOT_API __declspec(dllimport)


extern "C"
{
	SCREENSHOT_API int fnScreenshot(void);
	SCREENSHOT_API BITMAP fnScreenshotGetBitmap(void);
	SCREENSHOT_API int fnScreenshotCopyBitmap(char *pBuffer);
};

#endif