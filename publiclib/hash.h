#ifndef __HASH1_H__
#define __HASH1_H__

#include "def.h"

EXPORT_FUNCTION char* GetFileSHA1(wchar_t *FileNameInPut);
EXPORT_FUNCTION char* GetFileSHA1(char *FileNameInPut);

EXPORT_FUNCTION char* GetFileSHA1_CPP(wchar_t* FileNameInPut);

#endif