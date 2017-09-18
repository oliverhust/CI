#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

#include "inc.h"
#include "CA_Result.h"
#include "Git_MemConf.h"

//CA是否完成
static int g_CAResult = ERROR_FAILED;

//设置CA Lint中的Error,Warning,Info限制，超过限制值则不允许merge
static int g_limit_error[GRADE_LINT] = { 0 };
static int g_limit_warning[GRADE_LINT] = { 0 };
static int g_limit_info[GRADE_LINT] = { 0 };
static int g_limit_note[GRADE_LINT] = { 0 };

/* 设置CA正在忙碌标记 */
void CA_SetBusy()
{
	g_CAResult = ERROR_BUSY;
	return;
}

/* 获取CA是否完成标记 注意：客户端至少在被hook触发1秒后才能查询 */
errno_t CA_GetCAResult()
{
	return g_CAResult;
}

/* 获取变量数组的地址  */
void* CA_GetVariAddress(CA_RESULT_TYPE_E enType)
{
	void *pv;
	
	if (CA_RESULT_TYPE_MAX <= enType)
	{
		return NULL;
	}

	switch (enType)
	{
	case CA_RESULT_TYPE_ERROR:
		pv = g_limit_error;
		break;
	case CA_RESULT_TYPE_WARNING:
		pv = g_limit_warning;
		break;
	case CA_RESULT_TYPE_INFO:
		pv = g_limit_info;
		break;
	case CA_RESULT_TYPE_NOTE:
		pv = g_limit_note;
		break;
	default:
		pv = NULL;
		break;

	}

	return pv;
}

/* 获取变量的大小 */
int CA_GetVariSize(CA_RESULT_TYPE_E enType)
{
	int size = 0;

	if (CA_RESULT_TYPE_MAX <= enType)
	{
		return NULL;
	}

	switch (enType)
	{
	case CA_RESULT_TYPE_ERROR:
		size = sizeof(g_limit_error);
		break;
	case CA_RESULT_TYPE_WARNING:
		size = sizeof(g_limit_warning);
		break;
	case CA_RESULT_TYPE_INFO:
		size = sizeof(g_limit_info);
		break;
	case CA_RESULT_TYPE_NOTE:
		size = sizeof(g_limit_note);
		break;
	default:
		size = 0;
		break;

	}

	return size;
}


//根据字符串buf寻找pattern的位置并返回,buf最大为MAXLINE
int str_find_pos(char *buf, char *pattern)
{
	int i, len;

	len = strlen(pattern);
	for (i = 0; '\0' != buf[i]; i++)
	{
		if (0 == strncmp(buf + i, pattern, len))
		{
			return i;
		}
	}

	return LINE_MAXSIZE;
}

//从buf中获取error/warning/info的个数
int err_get_num(char *buf, int i)
{
	int j = i - 1;
	int n = 0;
	char c = buf[i];

	//从buf[i-1]往回读,直到遇到不是数字暂停.j和i分别指向数字两端的空格
	while (isdigit(buf[j]))
	{
		j--;
	}
	if (i - 1 - j > 4 || j == i - 1)
	{
		//太多错误了 几千个,或者读取不到错误个数
		return 9999;
	}

	buf[i] = '\0';
	n = atoi(buf + j + 1);
	buf[i] = c;

	return n;
}

//比较两个数组，前者的每个值都大于等于后者时返回真
BOOL err_array_cmp(int great[], int smaller[], int size)
{
	int i;
	BOOL bIsTrue = TRUE;
	for (i = 0; i < size; i++)
	{
		if (great[i] < smaller[i])
		{
			bIsTrue = FALSE;
			break;
		}
	}

	return bIsTrue;

}



/***********************************读取CA结果******************************************
由CA结果生成的文件判断是否通过CA 由开头标识"Note 900"判定
***************************************************************************************/
BOOL _CA_JudgeAndSetReulst()
{
	char buf[LINE_MAXSIZE];
	FILE *FL;
	errno_t err = 0;
	int num_err[GRADE_LINT] = { 0 }, num_warn[GRADE_LINT] = { 0 };
	int num_info[GRADE_LINT] = { 0 }, num_note[GRADE_LINT] = { 0 };
	int j, len;
	char *p_pattern = "Note 900";//特征字符
	BOOL b_result = FALSE;

	err = fopen_s(&FL, GitAutoCA_GetFileName(FILE_TYPE_CA_RESULT), "r");
	if (ERROR_SUCCESS != err || NULL == FL)
	{
		printf("Unable to open when read CA Result.\r\n");
		return FALSE;
	}

	j = 0;
	len = strlen(p_pattern);
	printf("Get CA Result:\r\n");
	//读取Error数量
	while (0 != fgets(buf, LINE_MAXSIZE, FL))
	{
		//查找有p_pattern开头的那一行
		if (0 == strncmp(buf, p_pattern, len))
		{
			//读取下一行到buf
			(void)fgets(buf, LINE_MAXSIZE, FL);
			//找到了含"Error,"开始提取出错误个数	
			num_err[j] = err_get_num(buf, str_find_pos(buf, " Error,"));
			num_warn[j] = err_get_num(buf, str_find_pos(buf, " Warning,"));
			num_info[j] = err_get_num(buf, str_find_pos(buf, " Info,"));
			num_note[j] = err_get_num(buf, str_find_pos(buf, " Note."));
			buf[LINE_MAXSIZE - 1] = '\0';
			printf("%s [%d,%d,%d,%d]\r\n", buf, num_err[j], num_warn[j], num_info[j], num_note[j]);
			j++;
		}

	}

	if (0 == j || GRADE_LINT < j)
	{
		return FALSE;
	}

	//根据数量来决定是否通过
	if (TRUE == err_array_cmp(g_limit_error, num_err, j) &&
		TRUE == err_array_cmp(g_limit_warning, num_warn, j) &&
		TRUE == err_array_cmp(g_limit_info, num_info, j) &&
		TRUE == err_array_cmp(g_limit_note, num_note, j))
	{
		b_result = TRUE;
	}

	fclose(FL);

	return b_result;
}

BOOL CA_JudgeAndSetReulst()
{
	if (TRUE == _CA_JudgeAndSetReulst())
	{
		g_CAResult = ERROR_SUCCESS;
		printf("Judge CA Result:Success\r\n");
		return TRUE;
	}

	g_CAResult = ERROR_FAILED;
	printf("Judge CA Result:Failed\r\n");
	return FALSE;
}

errno_t CA_WaitFinish()
{	
	return ERROR_SUCCESS;
}
