#ifndef __write_log_h__
#define __write_log_h__

#define WriteLog_PrintScreen 1

/* 设置路径 */
errno_t WriteLog_Init(const char *cPath);

/* 关闭文件 */
errno_t WriteLog_Fini();

/* 调用这个函数来打印Log到文件里 */
int WriteLog(const char *fmt, ...);

/* 空函数 不执行动作 */
int WriteLog_Null(const char *fmt, ...);

/*

#if WriteLogToFile
if (ERROR_SUCCESS != WriteLog_Init(PATH_WRITE_LOG))
{
return ERROR_FAILED;
}
#endif


#if WriteLogToFile
WriteLog_Fini();
#endif


//转移printf()到Log输出
#define WriteLogToFile 1

#if WriteLogToFile
#define printf WriteLog
#include "Write_Log.h"
#endif

*/


#endif