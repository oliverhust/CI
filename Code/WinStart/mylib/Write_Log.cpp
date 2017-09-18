#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "Write_Log.h"

#define ERROR_FAILED 1

FILE *fil;

errno_t WriteLog_Init(const char *cPath)
{
	errno_t err;

	err = fopen_s(&fil, cPath, "a");
	
	if (EXIT_SUCCESS != err)
	{
		return ERROR_FAILED;
	}

	return EXIT_SUCCESS;
}

errno_t WriteLog_Fini()
{
	return fclose(fil);
}

int WriteLog(const char *fmt, ...)
{
	va_list argptr;
	int ret;
	
	va_start(argptr, fmt);

#if WriteLog_PrintScreen
	vfprintf_s(stdout, fmt, argptr);
#endif
	ret = vfprintf_s(fil, fmt, argptr);

	va_end(argptr);

	return ret;
}

int WriteLog_Null(const char *fmt, ...)
{
	return strlen(fmt);
}