#include "stdafx.h"
#include <string.h>
#include <windows.h>

#include "Str.h"
#include "CA_Filter.h"
#include "CA_Filter_inc.h"
#include "CA_Run.h"

/* 执行当前目录下的CAalyzer.bat脚本 */
void CA_Run(IN int argc, IN _TCHAR* argv[], IN char *pcProgram, IN char *pcOutputFile)
{
	int i, iOffset = 0;
	char szArg[CA_FILTER_ARG_LEN] = { 0 }, szCmd[CA_FILTER_ARG_LEN] = { 0 };

	iOffset += sprintf_s(szCmd + iOffset, sizeof(szCmd) - iOffset, "%s ", pcProgram);

	for (i = 1; i < argc; ++i)
	{
		Str_Wchar2Char(argv[i], szArg, sizeof(szArg));
		iOffset += sprintf_s(szCmd + iOffset, sizeof(szCmd) - iOffset, "%s ", szArg);
	}

	//重定向输出到临时文件
	iOffset += sprintf_s(szCmd + iOffset, sizeof(szCmd) - iOffset, "> %s ", pcOutputFile);

	system(szCmd);

	return;
}