#ifndef __str_h__
#define __str_h__

/* 我的字符处理库  */

/* 路径分割符 */
#define MYSTR_PATH_SPILT '\\'

#define MYSTR_CACHE_SIZE 512

/* wchar*型字符串转char*型字符串 */
char *Str_Wchar2Char(wchar_t *WStr, char *CStr, int StrSize);

/* char*型字符串转wchar*型字符串 */
wchar_t *Str_Char2Wchar(char *CStr, wchar_t *WStr, int WStrSize);

/* 获取带路径的文件名的所在路径 末尾不带\ szInPath和szOutBase可以相同 */
char *Str_GetBasename_Char(char *szInPath, char* szOutBase, int OutSize);

/* 自带内存的字符串连接函数 */
char *Str_Cat(char *str1, char *str2);

/* 自带内存的任意个字符串连接函数 最后一个参数必须是0 */
char *Str_NCat(char *pcStr1, ...);

/* 打印文件内容 */
errno_t Str_PrintFile(char *pcPath);

#endif