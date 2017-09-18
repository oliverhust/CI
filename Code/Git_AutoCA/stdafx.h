// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

//#pragma comment(linker, "/subsystem:windows /entry:tmainCRTStartup")

//调试用，正常工作要注释掉
//#define system(x) puts(x)

//是否把所有printf记录到Log文件
#define WriteLogToFile 1

// TODO:  在此处引用程序需要的其他头文件
#if WriteLogToFile
#define printf WriteLog
#include "Write_Log.h"
#endif
