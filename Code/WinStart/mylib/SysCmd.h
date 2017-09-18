#ifndef __syscmd_h__
#define __syscmd_h__

/* 一行最大显示字符数 */
#define SYSCMD_STD_LINE_MAX				256

/* 最长路径名 */
#define SYSCMD_PATH_MAX					256

/* 设置系统命令执行结果的临时文件 */
errno_t SysCmd_SetResultFile(char *pcPath);

/* 获取系统命令的执行结果 */
errno_t SysCmd_Result(const char *pcCmd, char *pcOut, int size);

#endif