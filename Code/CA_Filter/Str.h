#ifndef __str_h__
#define __str_h__

/* 我的字符处理库  */

/* 如果需要用到正则相关的函数，则在工程中定义STR_USE_PCRE */
#define STR_USE_PCRE

/* 路径分割符 */
#define MYSTR_PATH_SPILT			'\\'

/* 分割行的标识符 */
#define STR_CHARACTER_NEWLINE		'\n'

/* 自带字体缓存的大小 */
#define MYSTR_CACHE_SIZE			512

/* 错误码定义 每个模块最多65535个 */
#define MODULE_ERROR_STR			0x00100000
#define STR_ERROR_TYPE_ERROR		(1UL << 12)
#define STR_ERROR_TYPE_WARNING		(2UL << 12)
#define STR_ERROR_TYPE_INFO			(3UL << 12)
#define ERROR_STR_NO_MATCH			(MODULE_ERROR_STR | STR_ERROR_TYPE_INFO | 0x001)

/* wchar*型字符串转char*型字符串 */
char *Str_Wchar2Char(wchar_t *WStr, char *CStr, int StrSize);

/* char*型字符串转wchar*型字符串 */
wchar_t *Str_Char2Wchar(char *CStr, wchar_t *WStr, int WStrSize);

/* 获取带路径的文件名的所在路径 末尾不带\ szInPath和szOutBase可以相同 */
char *Str_GetBasename_Char(char *szInPath, char* szOutBase, int OutSize);

/* 获取带路径名中的文件名 (与Str_GetBasename_Char功能相反)*/
char *Str_GetFilename_Char(char *szInPath, char* szOutFileName, int OutSize);

/* 自带内存的字符串连接函数 */
char *Str_Cat(char *str1, char *str2);

/* 自带内存的任意个字符串连接函数 最后一个参数必须是0 */
char *Str_NCat(char *pcStr1, ...);

/* 打印文件内容 */
errno_t Str_PrintFile(char *pcPath);

/* 如果pcPatt以pcHead开头则返回头部后面的字符串，否则返回NULL */
char *Str_BeginWith(char *pcPatt, char *pcHead);

/* 获取字符串的第一行到pcFirstLine,返回该行字符串的长度 */
int Str_Get_FirstLine(char *pcContent, char *pcFirstLine, int iFirstLineSize);

/* 字符串中是否按顺序出现某些字符串,最后一个参数必须是(char *)0,*/
/* 匹配返回最后一个匹配字符串末尾之后的字符串，不匹配返回NULL */
/* 如果参数含有空字符串""则跳过 */
char *Str_Contains_N(char *pcInput, ...);

//===============================正则式相关的函数==============================
#ifdef STR_USE_PCRE
#include "pcre.h"

/* 设置每次最大处理的字符串大小 */
#define STR_PCRE_MAXLINE					512

/* 设置最大的子串数 含\0 */
#define STR_PCRE_MAX_SUB					10	

/* Str_PcreModify的回调函数，入参为子串（\0,\1,\2..组成的字符串数组，除\0外其他可直接修改）,字符串的个数 */
typedef void(*STR_PCREMODIFY_PROC_PF)(char (*ppcSubStrings)[STR_PCRE_MAXLINE], int iStringNum);

/* 注册一条pcre的替换 返回pcre结构体指针(失败返回NULL) 入参：匹配正则式, pcre的编译option (如PCRE_UTF8..)*/
pcre *Str_PcreModify_Register(const char *pcPattern, int iOption);

/* 注册一条pcre的替换 入参：pcre结构体指针的地址 */
void Str_PcreModify_UnRegister(pcre *pstPcre);

/* pcre修改，修改匹配的子串\1,\2,\3.. */
/* 入参：pcre指针,Modify的回调函数, 输入文本，输出文本，输出文本的字符串大小 */
/* 匹配且修改成功返回ERROR_SUCCESS，未匹配返回ERROR_STR_NO_MATCH ，并拷贝原文到pcOut */
errno_t Str_PcreModify(pcre *pstPcre, STR_PCREMODIFY_PROC_PF pfModify, char *pcContent, char *pcOut, int size);


#endif

#endif