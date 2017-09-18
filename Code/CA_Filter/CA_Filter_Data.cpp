#include "stdafx.h"
#include <string.h>
#include <windows.h>

#include "CA_Filter.h"
#include "CA_Filter_inc.h"
#include "CA_Filter_Data.h"


static CA_FILTER_MD5_S g_astMD5[CA_FILTER_DATA_MD5_MAX] = { 0 };
static int g_iMD5Num = 0;		/* 当前存储的数量 */
static char g_aszFileName[CA_FILTER_DATA_FILENAME_MAX][CA_FILTER_FILENAME_MAX] = { 0 };
static int g_iFileNameNum = 0;	/* 当前存储的数量 */

/* 添加单条CA告警条目 MD5 */
errno_t CA_Filter_Data_Add_MD5(CA_FILTER_MD5_S *pstMD5)
{
	int i, iPos, iCmp = -1;

	/* 找到要插入的位置 */
	for (iPos = 0; iPos < g_iMD5Num; iPos++)
	{
		iCmp = _stricmp(pstMD5->szMD5, g_astMD5[iPos].szMD5);
		if (iCmp <= 0)
		{
			break;
		}

	}

	/* 防止重复添加 */
	if (0 == iCmp)
	{
		return ERROR_SUCCESS;
	}

	/* 该位置后面的往后移 */
	for (i = g_iMD5Num - 1; i >= iPos; i--)
	{
		memcpy(&g_astMD5[i + 1], &g_astMD5[i], sizeof(CA_FILTER_MD5_S));
	}

	/* 存入 */
	memcpy(&g_astMD5[iPos], pstMD5, sizeof(CA_FILTER_MD5_S));
	g_iMD5Num++;

	return ERROR_SUCCESS;
}

/* 删除单条CA告警条目 MD5 */
errno_t CA_Filter_Data_Del_MD5(CA_FILTER_MD5_S *pstMD5)
{
	int i, iPos;

	for (iPos = 0; iPos < g_iMD5Num; iPos++)
	{
		if (0 == _stricmp(pstMD5->szMD5, g_astMD5[iPos].szMD5))
		{
			/* 往前移动一位 */
			for (i = iPos; i <= g_iMD5Num - 2; i++)
			{
				memcpy(&g_astMD5[i], &g_astMD5[i + 1], sizeof(CA_FILTER_MD5_S));
			}
			g_iMD5Num--;
			memset(&g_astMD5[g_iMD5Num], 0, sizeof(CA_FILTER_MD5_S));

			break;
		}
	}

	return ERROR_SUCCESS;
}

/* 删除所有CA告警条目 MD5 */
void CA_Filter_Data_Del_MD5_All()
{
	memset(g_astMD5, 0, g_iMD5Num * sizeof(CA_FILTER_MD5_S));
	g_iMD5Num = 0;

}

/* 查询单条CA告警条目 （MD5）是否存在 */
errno_t CA_Filter_Data_IsExist_MD5(CA_FILTER_MD5_S *pstMD5)
{
	int low = 0, high = g_iMD5Num - 1, mid;
	errno_t err = ERROR_FAILED;
	BOOL bEq;

	while (low <= high)
	{
		mid = (low + high) / 2;
		bEq = _stricmp(g_astMD5[mid].szMD5, pstMD5->szMD5);
		if (0 == bEq)
		{
			err = ERROR_SUCCESS;
			break;
		}
		else if (bEq > 0)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}
	}

	return err;
}

/* 计算当前内存中保存的 MD5条目 总数 */
int CA_Filter_Data_Count_MD5()
{
	return g_iMD5Num;
}

/* 获取内存中第index个MD5条目的内容 */
errno_t CA_Filter_Data_Get_MD5(int index, CA_FILTER_MD5_S *pstCfg)
{
	if (0 <= index && index < g_iMD5Num)
	{
		memcpy(pstCfg, &g_astMD5[index], sizeof(CA_FILTER_MD5_S));
		return ERROR_SUCCESS;
	}

	return ERROR_FAILED;
}
/*=======================================================================================*/
/* 添加单条CA告警条目 文件名 */
errno_t CA_Filter_Data_Add_FileName(char *pcFileName)
{
	int i, iPos;

	if (NULL == pcFileName || strlen(pcFileName) >= CA_FILTER_FILENAME_MAX)
	{
		return ERROR_FAILED;
	}

	/* 找到要插入的位置 */
	for (iPos = 0; iPos < g_iFileNameNum; iPos++)
	{
		if (_stricmp(pcFileName, g_aszFileName[iPos]) < 0)
		{
			break;
		}

	}

	/* 该位置后面的往后移 */
	for (i = g_iFileNameNum - 1; i >= iPos; i--)
	{
		strcpy_s(g_aszFileName[i + 1], CA_FILTER_FILENAME_MAX, g_aszFileName[i]);
	}

	/* 存入 */
	strcpy_s(g_aszFileName[iPos], CA_FILTER_FILENAME_MAX, pcFileName);
	g_iFileNameNum++;

	return ERROR_SUCCESS;
}

/* 删除单条CA告警条目 文件名 */
errno_t CA_Filter_Data_Del_FileName(char *pcFileName)
{
	int i, iPos;

	if (NULL == pcFileName)
	{
		return ERROR_FAILED;
	}

	for (iPos = 0; iPos < g_iFileNameNum; iPos++)
	{
		if (0 == _stricmp(pcFileName, g_aszFileName[iPos]))
		{
			/* 往前移动一位 */
			for (i = iPos; i <= g_iFileNameNum - 2; i++)
			{
				strcpy_s(g_aszFileName[i], CA_FILTER_FILENAME_MAX, g_aszFileName[i + 1]);
			}
			g_iFileNameNum--;
			memset(g_aszFileName[g_iFileNameNum], 0, CA_FILTER_FILENAME_MAX);

			break;
		}
	}

	return ERROR_SUCCESS;
}

/* 删除所有CA告警条目 文件名 */
void CA_Filter_Data_Del_FileName_All()
{
	memset(g_aszFileName, 0, g_iFileNameNum * CA_FILTER_FILENAME_MAX);
	g_iFileNameNum = 0;

}

/* 查询单条CA告警条目 （文件名）是否存在 */
errno_t CA_Filter_Data_IsExist_FileName(char *pcFileName)
{
	int low = 0, high = g_iFileNameNum - 1, mid;
	errno_t err = ERROR_FAILED;
	BOOL bEq;

	if (NULL == pcFileName)
	{
		return ERROR_FAILED;
	}

	while (low <= high)
	{
		mid = (low + high) / 2;
		bEq = _stricmp(g_aszFileName[mid], pcFileName);
		if (0 == bEq)
		{
			err = ERROR_SUCCESS;
			break;
		}
		else if (bEq > 0)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}
	}

	return err;
	
}

/* 计算当前内存中保存的 文件名条目 总数 */
int CA_Filter_Data_Count_FileName()
{
	return g_iFileNameNum;
}

/* 获取内存中第index个 文件名条目 的内容 */
errno_t CA_Filter_Data_Get_FileName(int index, char pcFileName[CA_FILTER_FILENAME_MAX])
{
	if (0 <= index && index < g_iFileNameNum)
	{
		strcpy_s(pcFileName, CA_FILTER_FILENAME_MAX, g_aszFileName[index]);
		return ERROR_SUCCESS;
	}

	return ERROR_FAILED;
}


/*=======================================================================================*/
/*
int _tmain()
{
	int i;
	CA_FILTER_MD5_S stMD5;
	char szBuf[CA_FILTER_FILENAME_MAX];


#define ADD(x) strcpy_s(stMD5.szMD5, sizeof(stMD5.szMD5), x); CA_Filter_Data_Add_MD5(&stMD5);
#define DEL(x) strcpy_s(stMD5.szMD5, sizeof(stMD5.szMD5), x); CA_Filter_Data_Del_MD5(&stMD5);
#define DELALL() CA_Filter_Data_Del_MD5_All()
#define GET(x) strcpy_s(stMD5.szMD5, sizeof(stMD5.szMD5), x); \
	if (ERROR_SUCCESS == CA_Filter_Data_IsExist_MD5(&stMD5)) \
	{ \
	printf("Get %s Success\r\n", stMD5.szMD5); \
	}\
	else \
	{ \
	printf("Get %s Failed\r\n", stMD5.szMD5); \
	}
#define PRINTALL() \
	printf("\r\nPrint All Current Data:\r\n");\
	for (i = 0; i < CA_Filter_Data_Count_MD5(); i++)\
	{\
		CA_Filter_Data_Get_MD5(i, &stMD5);\
		printf("%s\r\n", stMD5.szMD5);\
	}\
	printf("\r\n");
	
	
#define ADD(x) CA_Filter_Data_Add_FileName(x)
#define DEL(x) CA_Filter_Data_Del_FileName(x)
#define DELALL() CA_Filter_Data_Del_FileName_All()
#define GET(x) if(ERROR_SUCCESS == CA_Filter_Data_IsExist_FileName(x))\
	{ \
		printf("Get %s Success\r\n", x); \
	}\
	else \
	{\
		printf("Get %s Failed\r\n", x); \
	}
#define PRINTALL() \
	printf("\r\nPrint All Current Data:\r\n"); \
	for (i = 0; i < CA_Filter_Data_Count_FileName(); i++)\
	{\
		CA_Filter_Data_Get_FileName(i, szBuf); \
		printf("%s\r\n", szBuf); \
	}\
	printf("\r\n");


	PRINTALL();
	ADD("CA_Filter_Data_IsExist_FileName_");
	PRINTALL();
	ADD("2A_Filter_Data_Get_FileName_cha2");
	PRINTALL();
	ADD("3A_Filter_Data_Get_FileName_cha3");
	PRINTALL();

	DEL("CA_Filter_Data_IsExist_FileName_");
	PRINTALL();
	DEL("2A_Filter_Data_Get_FileName_char");
	PRINTALL();
	DEL("3A_Filter_Data_Get_FileName_cha3");
	PRINTALL();
		//DELALL();
	printf("---------------------------------------\r\n");
	GET("1A_Filter_Data_Get_FileName_cha1");
	GET("2A_Filter_Data_Get_FileName_chaB");
	GET("2A_Filter_Data_Get_FileName_cha2");
	GET("3A_Filter_Data_Get_FileName_cha3");
	printf("---------------------------------------\r\n");

	DELALL();
	PRINTALL();

	(void)getchar();

}
*/
