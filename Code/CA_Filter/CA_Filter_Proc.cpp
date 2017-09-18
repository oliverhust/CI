#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "Str.h"

#include "CA_Filter.h"
#include "CA_Filter_inc.h"
#include "md5.h"
#include "CA_Run.h"
#include "CA_Filter_Pattern.h"
#include "CA_Filter_Proc.h"
#include "CA_Filter_Data.h"

/* 记录被过滤掉的各类告警数量 */
static int g_aiMessageHasMatch[CA_FILTER_MESSAGE_MAX] = { 0 };

static void _CA_Filter_Puts(char *pcSeg, FILE *stream)
{
	fprintf_s(stream, "%s\n", pcSeg);
	return;
}

/* 给定一个段，计算该段的`MD5`和文件名和错误类型,不符合条件则返回错误 */
static errno_t _CA_Filter_Proc_GetSegFeature(IN char *pcSeg, OUT CA_FILTER_SEG_GET_S *pstCfg)
{
	CA_FILTER_SEG_GET_S stCfg, stOut;
	int iOffset = 0, iOffTmp, iOutOff = 0;
	errno_t err = ERROR_SUCCESS, err_t;
	BOOL bHasMatch = BOOL_FALSE, bHasMD5 = BOOL_FALSE;
	char szLine[LINE_MAXSIZE] = { 0 }, szCalMD5[SEGMENT_MAXSIZE] = { 0 };
	
	while (0 != (iOffTmp = Str_Get_FirstLine(pcSeg + iOffset, szLine, sizeof(szLine))))
	{
		iOffset += iOffTmp;
		/* 分割成一行行进行处理 */
		/*iLineLen = pcLine + 1 - pcSeg;
		strncpy_s(szLine, sizeof(szLine), pcSeg + iOffset, iLineLen);
		iOffset += iLineLen;*/

#ifndef CA_FILTER_PROC_SHORTLINE
		/* 如果含有短行则整段跳过不提取 */
		if (BOOL_TRUE == CA_Filter_Pattern_IsShortLine(szLine))
		{
			err = ERROR_FAILED;
			break;
		}
#endif
		memset(&stCfg, 0, sizeof(stCfg));
		err_t = CA_Filter_Pattern_LineGetAndModify(szLine, &stCfg, szCalMD5 + iOutOff, sizeof(szCalMD5) - iOutOff);
		iOutOff = strlen(szCalMD5);
		/* 读到了一个未匹配的行 */
		if (ERROR_SUCCESS != err_t)
		{
			continue;
		}
		/* 读到一个匹配行 */
		bHasMatch = BOOL_TRUE;
		if (stCfg.szMD5[0] != '\0')
		{
			/* 如果读到行中本身包含MD5则直接以当前Cfg返回，不需要再读了 */
			bHasMD5 = BOOL_TRUE;
			memcpy(pstCfg, &stCfg, sizeof(stCfg));
			break;
		}
		memcpy(&stOut, &stCfg, sizeof(stCfg));

	}

	if (BOOL_FALSE == bHasMatch || ERROR_SUCCESS != err)
	{
		/* 全部行无一个符合条件,作为未识别的段 */
		return ERROR_FAILED;
	}
	else if (BOOL_TRUE != bHasMD5)
	{
		/* 无现成MD5,自己计算 */
		MD5_String(szCalMD5, strlen(szCalMD5), stOut.szMD5);
		memcpy(pstCfg, &stOut, sizeof(stOut));
	}

	return ERROR_SUCCESS;
}

/* 被去除末尾空行的段  普通Seg段处理：解析特征（MD5,文件名）查询数据库决定输出 统计告警 */
void CA_Filter_Proc_Seg(char *pcSeg, FILE *stream)
{
	CA_FILTER_SEG_GET_S stCfg;
	CA_FILTER_MD5_S stMD5Data;

	memset(&stCfg, 0, sizeof(stCfg));
	memset(&stMD5Data, 0, sizeof(stMD5Data));

	//计算该段的`MD5`和文件名和错误类型  
	if (ERROR_SUCCESS != _CA_Filter_Proc_GetSegFeature(pcSeg, &stCfg))
	{
		_CA_Filter_Puts(pcSeg, stream);
		return;
	}
	
	//查询数据库决定是否输出 都没有则直接Puts退出
	strcpy_s(stMD5Data.szMD5, CA_FILTER_MD5_STR_MAX, stCfg.szMD5);
	if (ERROR_SUCCESS != CA_Filter_Data_IsExist_MD5(&stMD5Data) && 
		ERROR_SUCCESS != CA_Filter_Data_IsExist_FileName(stCfg.szFileName))
	{
		_CA_Filter_Puts(pcSeg, stream);
		return;
	}

	//统计忽略的告警类型及数量
	g_aiMessageHasMatch[stCfg.enType]++;
}

void _CA_Filter_Proc_Modify_Note(char(*ppcSubStrings)[STR_PCRE_MAXLINE], int iStringNum)
{
	int iErr, iWarn, iInfo, iNote;

	iErr = atoi(ppcSubStrings[2]) - g_aiMessageHasMatch[CA_FILTER_MESSAGE_ERROR];
	iWarn = atoi(ppcSubStrings[3]) - g_aiMessageHasMatch[CA_FILTER_MESSAGE_WARNING];
	iInfo = atoi(ppcSubStrings[4]) - g_aiMessageHasMatch[CA_FILTER_MESSAGE_INFO];
	iNote = atoi(ppcSubStrings[5]) - g_aiMessageHasMatch[CA_FILTER_MESSAGE_NOTE];

	sprintf_s(ppcSubStrings[1], STR_PCRE_MAXLINE, "%d", iErr + iWarn + iInfo + iNote);
	sprintf_s(ppcSubStrings[2], STR_PCRE_MAXLINE, "%d", iErr);
	sprintf_s(ppcSubStrings[3], STR_PCRE_MAXLINE, "%d", iWarn);
	sprintf_s(ppcSubStrings[4], STR_PCRE_MAXLINE, "%d", iInfo);
	sprintf_s(ppcSubStrings[5], STR_PCRE_MAXLINE, "%d", iNote);	

	return;
}

/* Note信息处理：根据原来的统计信息修改 输入为‘两段’合成一段的段 */
void CA_Filter_Proc_Note(char *pcSeg, FILE *stream)
{
	int aiOrigin[CA_FILTER_MESSAGE_MAX] = { 0 }, iOffset = 0;
	char szLine[LINE_MAXSIZE] = { 0 }, szNew[LINE_MAXSIZE] = { 0 };
	char *pcXuXian = "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
	
	//打印第一行,取出第二行
	iOffset += Str_Get_FirstLine(pcSeg + iOffset, szLine, sizeof(szLine));
	fprintf(stream, "%s", szLine);
	iOffset += Str_Get_FirstLine(pcSeg + iOffset, szLine, sizeof(szLine));

	//获取和修改原来的告警类型及数量
	CA_Filter_Pattern_ModifyNote(_CA_Filter_Proc_Modify_Note, szLine, szNew, sizeof(szNew));
	fprintf(stream, "%s", szNew);

	//不显示各种告警的统计信息，而是用虚线代替
	fprintf(stream, "\n%s\n", pcXuXian);

	//清除原统计信息
	memset(g_aiMessageHasMatch, 0, sizeof(g_aiMessageHasMatch));

	return;
}

/* 输入一个段，学习这个段，无法识别则返回ERROR_FAILED */
errno_t CA_Filter_Proc_Study(char *pcSeg)
{
	CA_FILTER_SEG_GET_S stCfg;
	CA_FILTER_MD5_S stMD5;
	errno_t err;

	memset(&stCfg, 0, sizeof(stCfg));
	memset(&stMD5, 0, sizeof(stMD5));

	if (ERROR_SUCCESS != _CA_Filter_Proc_GetSegFeature(pcSeg, &stCfg))
	{
		return ERROR_FAILED;
	}

	strcpy_s(stMD5.szMD5, CA_FILTER_MD5_STR_MAX, stCfg.szMD5);
	err = CA_Filter_Data_Add_MD5(&stMD5);

	return err;
}

/* 解析配置文件，存到内存数据库中 */
errno_t CA_Filter_Proc_PraseCfgFile(IN char *pcLine)
{
	errno_t err = ERROR_FAILED;
	CA_FILTER_MD5_S stMD5;

	if (NULL != Str_BeginWith(pcLine, CA_FILTER_CFG_COMMENT))
	{
		return ERROR_SUCCESS;
	}
	else if (NULL != Str_BeginWith(pcLine, CA_FILTER_CFG_FILENAME))
	{
		/* 添加到数据库 要忽略的文件 */
		err = CA_Filter_Data_Add_FileName(pcLine + strlen(CA_FILTER_CFG_FILENAME));
	}
	else if (CA_FILTER_MD5_STR_MAX == strlen(pcLine) + 1)
	{
		/* 添加到数据库 MD5 */
		memset(&stMD5, 0, sizeof(stMD5));
		strcpy_s(stMD5.szMD5, sizeof(stMD5.szMD5), pcLine);
		err = CA_Filter_Data_Add_MD5(&stMD5);
	}
	return err;
}

/* 读取一个文件，文件每行均为要忽略CA告警的代码文件，存入数据库 */
void CA_Filter_Proc_IgnoreFile(FILE *fil)
{
	int iLen;
	char szLine[LINE_MAXSIZE] = { 0 };

	while (0 != fgets(szLine, sizeof(szLine), fil))
	{
		/* 去除空行 */
		if (BOOL_TRUE == CA_Filter_Pattern_IsAllSpace(szLine))
		{
			continue;
		}
		
		/* 去除末尾的换行符 */
		iLen = strlen(szLine);
		if ('\n' == szLine[iLen - 1])
		{
			szLine[iLen - 1] = '\0';
			if (2 <= iLen && '\r' == szLine[iLen - 2])
			{
				szLine[iLen - 2] = '\0';
			}
		}		

		/* 解析配置文件，存到内存数据库中 */
		(void)CA_Filter_Proc_PraseCfgFile(szLine);
		
	}

	return;
}

/* 保存当前内容中的所有配置到流中 */
errno_t CA_Filter_Proc_SaveCfgFile(FILE *fil)
{
	int i, iCount;
	CA_FILTER_MD5_S stMD5;
	char szFileName[CA_FILTER_FILENAME_MAX] = { 0 };	

	memset(&stMD5, 0, sizeof(stMD5));

	/* 打印忽略的文件名 */
	fprintf_s(fil, "%s\n", CA_FILTER_CFG_COMMENT_FILE_HEAD);
	iCount = CA_Filter_Data_Count_FileName();
	for (i = 0; i < iCount; i++)
	{
		CA_Filter_Data_Get_FileName(i, szFileName);
		fprintf_s(fil, "%s%s\n", CA_FILTER_CFG_FILENAME, szFileName);
	}
	fprintf_s(fil, "\n");
	
	/* 打印MD5 */
	fprintf_s(fil, "%s\n", CA_FILTER_CFG_COMMENT_MD5_HEAD);
	iCount = CA_Filter_Data_Count_MD5();
	for (i = 0; i < iCount; i++)
	{
		CA_Filter_Data_Get_MD5(i, &stMD5);
		fprintf_s(fil, "%s\n", stMD5.szMD5);
	}
	fprintf_s(fil, "\n");	

	return ERROR_SUCCESS;
}