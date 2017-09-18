#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "Str.h"
#include "pcre.h"

#include "CA_Filter.h"
#include "CA_Filter_inc.h"
#include "CA_Filter_Pattern.h"

//#pragma comment(lib, "pcre.lib") 

#define CA_FILTER_PCRE_OVECT_NUM				30

static pcre *g_pstReBasic, *g_pstReMD5, *g_pstModifyNote;
static pcre_extra* g_pcExBasic, *g_pcExMD5;

static char *g_apcCA_Filter_MessageType[CA_FILTER_MESSAGE_MAX] = { "Error", "Warning", "Info", "Note" };

/* 识别当前行是否有基本信息(文件名，告警类型) 的初始化 */
static errno_t _CA_Filter_Pattern_BasicInfo_Init()
{
	int iErroffset;
	const char* pcError;
	char *pcPattern = "\\s([C-Zc-z_]:.*) [lL](\\d+) (\\w+) (\\d+)";

	/* 编译正则式 */
	g_pstReBasic = pcre_compile(pcPattern, PCRE_UTF8, &pcError, &iErroffset, NULL);
	if (NULL == g_pstReBasic)
	{
		printf("compile regex expression \'%s\' error.\r\n", pcPattern);
		return ERROR_FAILED;
	}

	/* 对编译结果进行学习 */
	g_pcExBasic = pcre_study(g_pstReBasic, 0, &pcError);

	return ERROR_SUCCESS;
}

/* 识别当前行是否有MD5信息 的初始化 */
static errno_t _CA_Filter_Pattern_MD5_Init()
{
	int iErroffset;
	const char* pcError;
	char *pcPattern = "\\s[C-Zc-z_]:.* [lL]\\d+ \\w+ \\d+.*\\[([0-9a-fA-F]{32})\\]";

	/* 编译正则式 */
	g_pstReMD5 = pcre_compile(pcPattern, PCRE_UTF8, &pcError, &iErroffset, NULL);
	if (NULL == g_pstReMD5)
	{
		printf("compile regex expression \'%s\' error.\r\n", pcPattern);
		return ERROR_FAILED;
	}

	/* 对编译结果进行学习 */
	g_pcExMD5 = pcre_study(g_pstReMD5, 0, &pcError);

	return ERROR_SUCCESS;
}

errno_t _CA_Filter_Modify_Note_Init()
{
	char *pcPattern = "Total (\\d+) message.* (\\d+) Error, (\\d+) Warning, (\\d+) Info, (\\d+) Note";

	g_pstModifyNote = Str_PcreModify_Register(pcPattern, PCRE_UTF8);
	if (NULL != g_pstModifyNote)
	{
		return ERROR_SUCCESS;
	}

	return ERROR_FAILED;
}

/* 匹配的准备:预编译正则式 */
errno_t CA_Filter_Pattern_Init()
{
	errno_t err = ERROR_SUCCESS;

	err |= _CA_Filter_Pattern_BasicInfo_Init();
	err |= _CA_Filter_Pattern_MD5_Init();
	err |= _CA_Filter_Modify_Note_Init();

	return err;
}

/* 匹配结束，释放内存 */
void CA_Filter_Pattern_Fini()
{
	pcre_free(g_pstReBasic);
	pcre_free_study(g_pcExBasic);

	pcre_free(g_pstReMD5);
	pcre_free_study(g_pcExMD5);

	Str_PcreModify_UnRegister(g_pstModifyNote);

	return;
}

/* 未匹配行的处理 消除数字、空格 */
static void _ca_Proc_NotMatchLine(char *pcINLine, char *pcOut, int iOutSize)
{
	int iIN = 0, iOut = 0;
	unsigned char c;

	if (NULL == pcINLine || NULL == pcOut)
	{
		return;
	}

	while (NULL != (c = (unsigned char)pcINLine[iIN]))
	{
		/* 不是数字且不是空格就加进去 */
		if (!isdigit(c) && !isspace(c))
		{
			if (iOut >= iOutSize - 1)
			{
				break;
			}
			pcOut[iOut] = c;
			iOut++;
		}
		iIN++;
	}
	pcOut[iOut] = '\0';

	return;
}


/* 行识别获取信息:文件名 MD5(无则为"") 告警类型，并输出去掉行号的字符串 */
errno_t CA_Filter_Pattern_LineGetAndModify(char *pcContent, CA_FILTER_SEG_GET_S *pstCfg, char *pcOut, int size)
{
	int aiOvector[CA_FILTER_PCRE_OVECT_NUM] = { 0 }, iRc, iLen, i, iLineNumBegin, iLineNumFinNext;
	char szSub[LINE_MAXSIZE] = { 0 }, szID[CA_FILTER_MESS_ID_MAX] = { 0 };

	iLen = strlen(pcContent);

	//获取基本信息,记下行号位置
	iRc = pcre_exec(g_pstReBasic, g_pcExBasic, pcContent, iLen, 0, 0, aiOvector, sizeof(aiOvector) / sizeof(int));
	if (iRc < 0)
	{
		/* 未匹配的情况 */
		_ca_Proc_NotMatchLine(pcContent, pcOut, size);
		return ERROR_FAILED;
	}

	//把需要的捕获组拷贝出来进行处理
	//文件名
	pcre_copy_substring(pcContent, aiOvector, iRc, 1, szSub, sizeof(szSub));
	Str_GetFilename_Char(szSub, szSub, sizeof(szSub));
	strcpy_s(pstCfg->szFileName, CA_FILTER_FILENAME_MAX, szSub);
	//行号信息
	iLineNumBegin = aiOvector[2 * 2 + 0];
	iLineNumFinNext = aiOvector[2 * 2 + 1];
	//告警类型
	pcre_copy_substring(pcContent, aiOvector, iRc, 3, szSub, sizeof(szSub));
	for (i = 0; i < CA_FILTER_MESSAGE_MAX; i++)
	{
		if (0 == strcmp(g_apcCA_Filter_MessageType[i], szSub))
		{
			pstCfg->enType = (CA_FILTER_MESSAGE_E)i;
			break;
		}
	}
	/* 找不到这种告警类型 Error,Info,... */
	if (CA_FILTER_MESSAGE_MAX == i)
	{
		_ca_Proc_NotMatchLine(pcContent, pcOut, size);
		return ERROR_FAILED;
	}
	//告警ID
	pcre_copy_substring(pcContent, aiOvector, iRc, 4, szID, sizeof(szID));

	//尝试获取MD5
	memset(aiOvector, 0, sizeof(aiOvector));
	iRc = pcre_exec(g_pstReMD5, g_pcExMD5, pcContent, iLen, 0, 0, aiOvector, sizeof(aiOvector) / sizeof(int));
	if (0 <= iRc)
	{
		pcre_copy_substring(pcContent, aiOvector, iRc, 1, pstCfg->szMD5, CA_FILTER_MD5_STR_MAX);
	}

	//只输出文件名 告警类型 告警ID
	sprintf_s(pcOut, size, "%s %s %s\n", pstCfg->szFileName, g_apcCA_Filter_MessageType[pstCfg->enType], szID);

	return ERROR_SUCCESS;
}

/* 修改最后的统计信息 */
errno_t CA_Filter_Pattern_ModifyNote(STR_PCREMODIFY_PROC_PF pfModify, char *pcContent, char *pcOut, int size)
{
	return Str_PcreModify(g_pstModifyNote, pfModify, pcContent, pcOut, size);
}

/* “有效”字符在2~6个及之间则为“短行” */
BOOL CA_Filter_Pattern_IsShortLine(char *pcPatt)
{
	int i, iLen, iCount = 0;

	iLen = strlen(pcPatt);

	for (i = 0; i < iLen; i++)
	{
		if (!isspace((unsigned char)pcPatt[i]))
		{
			iCount++;
		}
	}

	if (CA_FILTER_PATTERN_SHORT_MIN <= iCount && iCount <= CA_FILTER_PATTERN_SHORT_MAX)
	{
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}

/* 判断是否Note900后一段的各种告警类型 */
BOOL CA_Filter_Pattern_IsAfterMessage(char *pcPatt)
{
	char szLine[LINE_MAXSIZE] = { 0 };

	Str_Get_FirstLine(pcPatt, szLine, sizeof(szLine));

	if (NULL != Str_Contains_N(szLine, "Count", "Num", "Type", (char *)0) || 
		NULL != Str_Contains_N(szLine, "Count", "No", "Type", (char *)0))
	{
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}

/* 是否全部为空格字符 */
BOOL CA_Filter_Pattern_IsAllSpace(char *pcPatt)
{
	int i, iLen;
	BOOL bRet = BOOL_TRUE;

	iLen = strlen(pcPatt);

	for (i = 0; i < iLen; i++)
	{
		if (!isspace((unsigned char)pcPatt[i]))
		{
			bRet = BOOL_FALSE;
			break;
		}
	}

	return bRet;
}


/* 是否可以直接打印而不需处理 CA告警边界，回车 */
BOOL CA_Filter_Pattern_IsPutsDirect(char *pcPatt)
{
	/* 如果全部是空格组成则是 */
	if (BOOL_TRUE == CA_Filter_Pattern_IsAllSpace(pcPatt))
	{
		return BOOL_TRUE;
	}
	
	if (NULL != Str_BeginWith(pcPatt, "--- END") ||
		NULL != Str_BeginWith(pcPatt, "--- Lintware") ||
		NULL != Str_BeginWith(pcPatt, "--- Codeware") ||
		NULL != Str_BeginWith(pcPatt, "--- Checkware"))
	{
		return BOOL_TRUE;
	}

	if (NULL != strstr(pcPatt, "CAnalyzer") && 
	   (NULL != Str_BeginWith(pcPatt, "---") || NULL != Str_BeginWith(pcPatt, "--- BEG")))
	{
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}

/* 判断是否为Note信息 */
BOOL CA_Filter_Pattern_IsNote(char *pcPatt)
{
	if (NULL != Str_BeginWith(pcPatt, "Note 900") &&
		NULL != Str_Contains_N(pcPatt, "\n", "Total", "message", "Error", "Warning", "Info", "Note", (char *)0))
	{
		return BOOL_TRUE;
	}
	return BOOL_FALSE;
}


/*
static char *pcContent = \
"*** E:\\SVTI\\Source\\NETFWD\\src\\sbin\\tunnel\\xmluiplugin\\tunnel_xml_tunnels.c L753 Warning 426 _xml_TunnelProcTlvMsg(?, ?, !=0, !=0) \r\n"
"    E:\\SVTI\\Source\\NETFWD\\src\\sbin\\tunnel\\xmluiplugin\\tunnel_xml_tunnels.c L676 Warning 426[_xml_TunnelProcTlvMsg][6FE139CB319B099894F74038C800CB1A]\r\n"
"Call to function 'TLV_CreateReceiveMsg(unsigned int, void *, unsigned short)' violates semantic '2p' \r\n";
static char *pcContent1 = "Hello, world!\r\n";

int _tmain(int argc, char* argv[])
{
	pcre *pstRe;
	pcre_extra* pcEx;
	int aiOvector[30], iErroffset, iRc;
	const char* pcError;
	char *pcPattern = "\\s([C-Zc-z_]:.*) [lL]([0-9][0-9]*) (\\w+) (\\d+)";
	char szSub[LINE_MAXSIZE] = { 0 };

	int i, iOffset = 0;

	// 编译正则式 
	pstRe = pcre_compile(pcPattern, PCRE_MULTILINE | PCRE_UTF8, &pcError, &iErroffset, NULL);
	if (NULL == pstRe)
	{
		printf("compile regex expression \'%s\' error.\r\n", pcPattern);
		return ERROR_FAILED;
	}

	// 对编译结果进行学习 
	pcEx = pcre_study(pstRe, PCRE_STUDY_JIT_COMPILE, &pcError);

	// 获取多个匹配结果 
	// 正则式匹配,返回匹配的偏移位置 
	while (0 <= (iRc = pcre_exec(pstRe, pcEx, pcContent, strlen(pcContent), iOffset, 0, aiOvector, sizeof(aiOvector) / sizeof(int))))
	{
		printf("Match Regex Expression:\r\n");
		// 读取出捕获的子串 
		for (i = 0; i <= 4; ++i)
		{
			pcre_copy_substring(pcContent, aiOvector, iRc, i, szSub, sizeof(szSub));
			printf("Sub%d=%s\r\n", i, szSub);
			//打印匹配子串的起始字节位置
			printf("aiOvector[%d]=%d\r\n", 2 * i, aiOvector[2 * i]);
			//打印匹配子串末尾再后面一个字节的位置
			printf("aiOvector[%d]=%d\r\n", 2 * i + 1, aiOvector[2 * i + 1]);

		}

		// 设置偏移，准备下一次匹配 
		iOffset = aiOvector[1];

		printf("\r\n");
	}

	pcre_free(pstRe);
	system("pause");
	return 0;
}
*/

/* //原版的代码
int _tmain(int argc, char* argv[])
{
	char pattern[] = "\\d+";
	char content[] = "owaefj ifea 894 aewoai 43";
	const char* error;
	int erroffset;
	int cur = 0;
	int ovector[30];
	pcre *re;

	re = pcre_compile(pattern, PCRE_MULTILINE | PCRE_NO_AUTO_CAPTURE | PCRE_UTF8, &error, &erroffset, NULL);

	pcre_extra* pcEx = pcre_study(re, PCRE_STUDY_JIT_COMPILE, &error);
	int rc = pcre_exec(re, pcEx, content, strlen(content), cur, PCRE_NOTEMPTY, ovector, sizeof(ovector)/sizeof(int));

	if (rc >= 0)
	{
		char* ptr;
		ptr = content + ovector[1];
		*ptr = '\0';
		printf("%s\n", content + ovector[0]);
	}

	pcre_free(re);
	system("pause");
	return 0;
}
*/
