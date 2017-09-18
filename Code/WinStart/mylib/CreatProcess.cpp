#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>

static wchar_t *Mystr_Char2Wchar(char *CStr, wchar_t *WStr, int WStrSize)
{
	size_t len = strlen(CStr) + 1;
	size_t converted = 0;

	if ((unsigned int)WStrSize < len * sizeof(wchar_t))
	{
		return NULL;
	}

	mbstowcs_s(&converted, WStr, len, CStr, _TRUNCATE);

	WStr[WStrSize / sizeof(wchar_t)-1] = 0;

	return WStr;
}

errno_t NewProcess(char *cmd)
{
	STARTUPINFO siStartInfo;
	PROCESS_INFORMATION piProcInfo;
	WCHAR wbuf[1024] = { 0 };
	errno_t err;

	//设置进程启动信息
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(siStartInfo);
	siStartInfo.wShowWindow = SW_HIDE;

	//将char *cmd 转换为wcahr型
	Mystr_Char2Wchar(cmd, wbuf, sizeof(wbuf));

	//新建进程
	err = CreateProcess(NULL,
		wbuf,
		NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_NO_WINDOW, NULL,
		NULL,
		&siStartInfo, &piProcInfo);

	return err;
}