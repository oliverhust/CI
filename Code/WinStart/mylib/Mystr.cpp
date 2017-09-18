#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "Mystr.h"

#define ERROR_SUCCESS							0
#define ERROR_FAILED							1
#define STR_READ_FILE_EACH_SIZE					4096

char StrCache[MYSTR_CACHE_SIZE] = { 0 };

char *Mystr_Wchar2Char(wchar_t *WStr, char *CStr, int StrSize)
{
	size_t len = wcslen(WStr) + 1;
	size_t converted = 0;

	if ((unsigned int)StrSize < len * sizeof(char))
	{
		return NULL;
	}

	wcstombs_s(&converted, CStr, len, WStr, _TRUNCATE);

	CStr[StrSize - 1] = '\0';

	return CStr;
}

wchar_t *Mystr_Char2Wchar(char *CStr, wchar_t *WStr, int WStrSize)
{
	size_t len = strlen(CStr) + 1;
	size_t converted = 0;

	if ((unsigned int)WStrSize < len * sizeof(wchar_t))
	{
		return NULL;
	}

	mbstowcs_s(&converted, WStr, len, CStr, _TRUNCATE);

	WStr[WStrSize / sizeof(wchar_t) - 1] = 0;

	return WStr;
}

char *Mystr_GetBasename_Char(char *szInPath, char* szOutBase, int OutSize)
{
	int i, len;

	len = strlen(szInPath);
	if (OutSize < len + 1)
	{
		return NULL;
	}

	for (i = len - 1; i >= 0; i--)
	{
		if (MYSTR_PATH_SPILT == szInPath[i])
		{
			break;
		}
	}
	
	if (0 < i)
	{
		memcpy(szOutBase, szInPath, i);
		szOutBase[i] = '\0';
	}
	else
	{
		szOutBase[0] = '\0';
	}
	

	return szOutBase;
}

char *Mystr_Cat(char *str1, char *str2)
{
	int i = 0;

	i += sprintf_s(MystrCache + i, sizeof(MystrCache) - i, "%s", str1);
	i += sprintf_s(MystrCache + i, sizeof(MystrCache) - i, "%s", str2);

	return MystrCache;

}

char *Mystr_NCat(char *pcStr1, ...)
{
	va_list stAp;
	int off = 0;
	char *pcArgs = pcStr1;

	memset(MystrCache, 0, sizeof(MystrCache));

	/* 初始化（需要获取...前的第一个参数） */
	va_start(stAp, pcStr1);

	while (NULL != pcArgs)
	{
		/* 将各个参数打印到全局变量 */
		off += sprintf_s(MystrCache + off, sizeof(MystrCache) - off, "%s", pcArgs);

		/* 获取下一个参数 */
		pcArgs = va_arg(stAp, char *);
	}

	va_end(stAp);

	return MystrCache;
}

/* 打印文件内容 */
errno_t Str_PrintFile(char *pcPath)
{
	FILE *fil;
	int iRead;
	char szBuf[STR_READ_FILE_EACH_SIZE + 1] = { 0 };

	if (0 != fopen_s(&fil, pcPath, "r"))
	{
		printf("Open file %s error.\r\n", pcPath);
		return ERROR_FAILED;
	}

	/* 每次读取4096字节 */
	while (0 != (iRead = fread(szBuf, sizeof(char), sizeof(szBuf) - 1, fil)))
	{
		szBuf[iRead] = '\0';
		printf("%s", szBuf);
	}
	printf("\r\n");

	fclose(fil);

	return ERROR_SUCCESS;
}

/* 测试 */
/*
int main(int argc, char *argv[])
{
	char c;

	puts(Mystr_NCat("You ", "are ", "a ", "pig ", "!!!", NULL));
	puts(Mystr_NCat("Hello", NULL));
	puts(Mystr_NCat(NULL));
	puts(Mystr_NCat("1234567"));
	c = 'Q';
	putchar(c);

	printf("============Str_FilePrint Test================\r\n");
	Str_PrintFile("CA_Result_Origin.txt");
	printf("============Str_FilePrint Test Finish================\r\n");

	(void)getchar();

	return 0;
}
*/