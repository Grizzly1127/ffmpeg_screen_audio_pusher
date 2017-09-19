#pragma once
#include "def.h"

class EXPORT_FUNCTION ToolEx
{
public:
	ToolEx();
	~ToolEx();

public:
	bool AlreadyRunning(LPCWSTR strAppName);
};

