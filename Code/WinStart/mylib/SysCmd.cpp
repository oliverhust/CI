#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>
#include <time.h>

#include "SysCmd.h"

#define ERROR_FAILED		1

static char g_szResultFile[SYSCMD_PATH_MAX] = { 0 };

errno_t SysCmd_SetResultFile(char *pcPath)
{
	strcpy_s(g_szResultFile, sizeof(g_szResultFile), pcPath);

	return ERROR_SUCCESS;
}

/* 执行系统命令并获取显示结果 */
errno_t SysCmd_Result(const char *pcCmd, char *pcOut, int size)
{
	FILE *fp;
	int off = 0;
	char szCmd[SYSCMD_STD_LINE_MAX] = { 0 };
	char szLine[SYSCMD_STD_LINE_MAX] = { 0 };

	if (NULL == pcOut)
	{
		system(pcCmd);
		return ERROR_SUCCESS;
	}
		
	/* 将结果重定向到文件 */
	sprintf_s(szCmd, sizeof(szCmd), "%s > %s", pcCmd, g_szResultFile);
	system(szCmd);

	/* 打开文件 */
	fopen_s(&fp, g_szResultFile, "r");
	if (NULL == fp)
	{
		printf("Open SysCmd result file error.\r\n");
		return ERROR_FAILED;
	}

	/* 读取文件 */
	while (fgets(szLine, sizeof(szLine), fp) != NULL)
	{
		printf("%s", szLine);
		off += sprintf_s(pcOut + off, size - off, "%s", szLine);
	}
	fclose(fp);

	/* 删除文件 */
	sprintf_s(szCmd, sizeof(szCmd), "del /F %s", g_szResultFile);
	system(szCmd);

	return ERROR_SUCCESS;
}