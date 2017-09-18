#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "Write_Log.h"

#define ERROR_SUCCESS		0
#define ERROR_FAILED		1

FILE *fil;
char g_szPath[WRITELOG_FILENAME_MAX] = { 0 };

static errno_t _WriteLog_Init()
{
	errno_t err;

	err = fopen_s(&fil, g_szPath, "a");

	if (ERROR_SUCCESS != err)
	{
		return ERROR_FAILED;
	}

	return ERROR_SUCCESS;
}

static errno_t _WriteLog_Fini()
{
	return fclose(fil);
}

errno_t WriteLog_Init(const char *cPath)
{
	errno_t err;

	strcpy_s(g_szPath, sizeof(g_szPath), cPath);
	err = _WriteLog_Init();

	return err;
}

errno_t WriteLog_Fini()
{
	return _WriteLog_Fini();
}

int WriteLog(const char *fmt, ...)
{
	va_list argptr;
	int ret;

	if (NULL == fmt)
	{
		puts("Write Log error");
		return 0;
	}

	va_start(argptr, fmt);

	if (NULL == argptr)
	{
		puts("Write Log error, argptr == NULL:");
		return 0;
	}

#if WriteLog_PrintScreen
	vfprintf_s(stdout, fmt, argptr);
#endif
	ret = vfprintf_s(fil, fmt, argptr);

	va_end(argptr);

	return ret;
}