// CA_Filter.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>
#include <time.h>
#include "Str.h"
#include "md5.h"

#include "CA_Filter.h"
#include "CA_Filter_inc.h"
#include "CA_Run.h"
#include "CA_Filter_Pattern.h"
#include "CA_Filter_Proc.h"
#include "CA_Filter_Data.h"

/* CA脚本的名称 */
static char g_szCAnalyzerBat[CA_FILTER_FILENAME_MAX] = { CA_FILTER_BAT_FILE };

/* 读取一段的内容 直到读到一个空行并丢弃 返回内容中不含空行 返回值为写入字符数 */
int _CA_Filter_ReadSegment(FILE *fil, char *pcSeg, int size)
{
	int iOffset = 0;
	char szLine[LINE_MAXSIZE] = { 0 };

	/* 读取一行然后判断是否空行 不是空行则循环读取 */
	while (0 != fgets(szLine, sizeof(szLine), fil) &&
		   !CA_Filter_Pattern_IsAllSpace(szLine))
	{
		strcpy_s(pcSeg + iOffset, size - iOffset, szLine);
		iOffset += strlen(szLine);
	}

	/* BUG修复:最后一行可能不包含换行符，添加换行符到段的末尾(防止最后一个段计算MD5时缺少\n导致计算错误) */
	if (iOffset >= 1 && pcSeg[iOffset - 1] != '\n')
	{
		strcpy_s(pcSeg + iOffset, size - iOffset, "\n");
		iOffset += strlen("\n");
	}

	return iOffset;

}

/* CA学习处理流程 */
errno_t _CA_Filter_Study(FILE *fil)
{
	int iOffset = 0 ;
	char szLine[LINE_MAXSIZE] = { 0 }, szSeg[SEGMENT_MAXSIZE] = { 0 };

	/* 跳过第一行(第一行可能包含Windows记事本自带的数据头) */
	if (0 == fgets(szLine, sizeof(szLine), fil))
	{
		return ERROR_SUCCESS;
	}
	
	/* 分段分类学习处理流程 */
	while (0 != fgets(szLine, sizeof(szLine), fil))
	{
		/* 如果是可以直接打印的行，则不进行学习 */
		if (BOOL_TRUE == CA_Filter_Pattern_IsPutsDirect(szLine))
		{
			continue;
		}

		/* 读取一段 */
		iOffset = sprintf_s(szSeg, sizeof(szSeg), "%s", szLine);
		iOffset += _CA_Filter_ReadSegment(fil, szSeg + iOffset, sizeof(szSeg) - iOffset);

		/* 普通Seg段处理：解析特征（MD5,文件名）查询数据库决定输出 统计告警 */
		/* 无法识别的段不学习 */
		(void)CA_Filter_Proc_Study(szSeg);

	}

	return ERROR_SUCCESS;

}


/* CA过滤处理流程 */
errno_t _CA_Filter_Proc(FILE *fil, FILE *stream)
{
	int iOffset = 0;
	BOOL bHasReadNextLine = BOOL_FALSE;
	char szLine[LINE_MAXSIZE] = { 0 }, szSeg[SEGMENT_MAXSIZE] = { 0 };

	/* 跳过第一行(第一行可能包含Windows记事本自带的数据头) */
	if (0 == fgets(szLine, sizeof(szLine), fil))
	{
		return ERROR_FAILED;
	}
	fprintf(stream, "%s", szLine);
	
	/* 分段分类过滤处理流程 */
	/* 如果需要暂时预读一行，则可以在处理流程中读取到szLine中然后置iOffset为预读取的长度 */
	while (BOOL_TRUE == bHasReadNextLine || 0 != fgets(szLine, sizeof(szLine), fil))
	{
		bHasReadNextLine = BOOL_FALSE;
		if (BOOL_TRUE == CA_Filter_Pattern_IsPutsDirect(szLine))
		{
			fprintf(stream, "%s", szLine);
			continue;
		}

		/* 读取一段，如果是Note900则再读取一段，并交给Note处理流程处理 */
		iOffset = sprintf_s(szSeg, sizeof(szSeg), "%s", szLine);
		iOffset += _CA_Filter_ReadSegment(fil, szSeg + iOffset, sizeof(szSeg) - iOffset);
		if (CA_Filter_Pattern_IsNote(szSeg))
		{
			/* Note信息处理：根据原来的统计信息修改 */
			CA_Filter_Proc_Note(szSeg, stream);

			/* 获取并识别下一行是否为相关信息, 不是则当做预读了下一行 */
			if (0 == fgets(szLine, sizeof(szLine), fil))
			{
				break;
			}
			
			if (BOOL_TRUE == CA_Filter_Pattern_IsAfterMessage(szLine))
			{
				/* 对于这些信息目前暂不处理，直接忽略也不打印 */
				_CA_Filter_ReadSegment(fil, szSeg, sizeof(szSeg));
			}
			else
			{
				/* 如果不是Note900相关信息则把当前预读了的行进行到下一次循环中 */
				bHasReadNextLine = BOOL_TRUE;
			}
			
			continue;
		}

		/* 普通Seg段处理：解析特征（MD5,文件名）查询数据库决定输出 统计告警 */
		/* 无法识别的段直接打印 */
		CA_Filter_Proc_Seg(szSeg, stream);

	}

	return ERROR_SUCCESS;

}

/* 初始化 */
errno_t CA_Filter_Init()
{
	if (ERROR_SUCCESS != CA_Filter_Pattern_Init())
	{
		return ERROR_FAILED;
	}

	return ERROR_SUCCESS;
}

/* 去初始化*/
void CA_Filter_Fini()
{
	CA_Filter_Pattern_Fini();

	return;
}

/* 根据标准的CA Lint文件进行学习 */
errno_t CA_Filter_Study(char *pcStandardFile)
{
	FILE *fil;
	errno_t err;

	if (ERROR_SUCCESS != fopen_s(&fil, pcStandardFile, "r") || NULL == fil)
	{
		printf("Open Study File %s Error.\r\n", pcStandardFile);
		return ERROR_FAILED;
	}

	err = _CA_Filter_Study(fil);
	fclose(fil);

	return err;
}

/* 读取配置文件，存入数据库 */
errno_t CA_Filter_ReadCfgFiles(char *pcFileNameFile)
{
	FILE *fil;

	if (ERROR_SUCCESS != fopen_s(&fil, pcFileNameFile, "r") || NULL == fil)
	{
		//printf("Read Cfg File %s Error.\r\n", pcFileNameFile);
		return ERROR_FAILED;
	}

	CA_Filter_Proc_IgnoreFile(fil);

	fclose(fil);

	return ERROR_SUCCESS;
}

/* 将数据库中的配置保存到文件中 */
errno_t _CA_Filter_SaveCfgFile(FILE *fil)
{
	return CA_Filter_Proc_SaveCfgFile(fil);
}

/* 将数据库中的配置保存到配置文件 */
errno_t CA_Filter_SaveCfgFiles(char *pcFileNameFile)
{
	FILE *fil;
	errno_t err;

	if (ERROR_SUCCESS != fopen_s(&fil, pcFileNameFile, "w") || NULL == fil)
	{
		printf("Write Cfg File %s Error.\r\n", pcFileNameFile);
		return ERROR_FAILED;
	}

	err = _CA_Filter_SaveCfgFile(fil);
	fclose(fil);

	return err;
}

/* 把CA_Lint文件内容进行过滤，结果存到STREAM中 */
errno_t _CA_Filter(char *pcSrcFile, FILE *stream)
{
	FILE *fil;
	errno_t err;
	int i;

	/* 多次尝试打开原CA告警文件 */
	for (i = 0; i < CA_FILTER_OPENORIGIN_TIMES; ++i)
	{
		if (ERROR_SUCCESS == fopen_s(&fil, pcSrcFile, "r") && NULL != fil)
		{
			break;
		}
		Sleep(CA_FILTER_OPENORIGIN_INTERVAL);
	}

	if (CA_FILTER_OPENORIGIN_TIMES == i)
	{
		printf("Open Origin File %s Error.\r\n", pcSrcFile);
		return ERROR_FAILED;
	}

	err = _CA_Filter_Proc(fil, stream);
	fclose(fil);

	return err;
}

/* 把CA_Lint文件内容进行过滤，结果存到另一个文件 */
errno_t CA_Filter(char *pcSrcFile, char *pcDstFile)
{
	FILE *fil;
	errno_t err;

	if (ERROR_SUCCESS != fopen_s(&fil, pcDstFile, "w") || NULL == fil)
	{
		printf("Open CA Filter Dst file %s error.\r\n", pcDstFile);
		return ERROR_FAILED;
	}

	err = _CA_Filter(pcSrcFile, fil);

	fclose(fil);

	return err;
}

/* 从内存中删除所有学习结果 */
errno_t CA_Filter_ClearAll()
{
	CA_Filter_Data_Del_MD5_All();
	CA_Filter_Data_Del_FileName_All();
	return ERROR_SUCCESS;
}

/* ====================生成CA_Filter.exe需要的函数================= */
#ifdef CA_FILTER_MAKE_EXE
static errno_t _Init(int argc, _TCHAR* argv[])
{
	errno_t err = ERROR_SUCCESS;
	char szArg0[CA_FILTER_FILENAME_MAX] = { 0 },
		 szTmp[CA_FILTER_FILENAME_MAX] = { 0 };
	

	//获取当前路径,并设置为当前工作目录
	Str_Wchar2Char(argv[0], szArg0, sizeof(szArg0));
	Str_GetBasename_Char(szArg0, szTmp, sizeof(szTmp));
	if (szTmp[0] != '\0' && ERROR_SUCCESS != _chdir(szTmp))
	{
		return ERROR_FAILED;
	}

	//如果当前程序的名字与MCA脚本的名字一样（除了后缀名），则进行MCA
	Str_GetFilename_Char(szArg0, szTmp, sizeof(szTmp));
	if (0 == strcmp(szTmp, CA_FILTER_MCA_EXE_NAME))
	{
		strcpy_s(g_szCAnalyzerBat, sizeof(g_szCAnalyzerBat), CA_FILTER_M_BAT_FILE);
	}
	else
	{
		strcpy_s(g_szCAnalyzerBat, sizeof(g_szCAnalyzerBat), CA_FILTER_BAT_FILE);
	}

	err = CA_Filter_Init();

	return err;
}

static void _Fini()
{
	CA_Filter_Fini();
	return;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (ERROR_SUCCESS != _Init(argc, argv))
	{
		printf("CA Filter Init Failed.\r\n");
		return ERROR_FAILED;
	}
	
	/* 读取配置文件 */
	(void)CA_Filter_ReadCfgFiles(CA_FILTER_READ_CFG_FILE);

	/* 学习 */
	(void)CA_Filter_Study(CA_FILTER_STUDY_FILE);

	/* 执行CA命令，并保存结果到文件CA_FILTER_ORIGIN_RESULT */
	CA_Run(argc, argv, g_szCAnalyzerBat, CA_FILTER_ORIGIN_RESULT);

		
	/* CA过滤，结果保存到CA_FILTER_NEW_RESULT */
	if (ERROR_SUCCESS != CA_Filter(CA_FILTER_ORIGIN_RESULT, CA_FILTER_NEW_RESULT))
	{
		printf("CA Filter Error.\r\n");
		_Fini();
		return ERROR_FAILED;
	}

	/* 打印结果 */
	(void)Str_PrintFile(CA_FILTER_NEW_RESULT);
	/* 将内存中的数据输出为配置文件 */
	CA_Filter_SaveCfgFiles(CA_FILTER_AUTOSAVE_CFGFILE);

	_Fini();

	return ERROR_SUCCESS;
}

#endif

