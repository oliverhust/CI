// Git_AutoCA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>
#include <time.h>

#include "inc.h"
#include "Str.h"
#include "status.h"
#include "Config.h"
#include "CA_Result.h"
#include "Git_MemConf.h"
#include "Write_Log.h"
#include "Server_Socket.h"
#include "Server_Proc.h"

static char *g_pcSuccessFailed[] = { "Success", "Failed", "Unknown" };

//打印当前时间
void print_time(void)
{
	time_t tim;
	char buf[LINE_MAXSIZE];
	time(&tim);
	ctime_s(buf, sizeof(buf), &tim);

	printf("%s", buf);

	return;
}

/* 等待编译结束 */
void _otherjobs_WaitFinish()
{
	printf("Wait for OtherJobs finish...\r\n");
	status_set("WaitOtherJobsFinish");
	while(ERROR_BUSYING == ServerProc_GetOtherJobsResult())
	{
		Sleep(1);
	}
	status_set("OtherJobs_finish");
	printf("Get signal of OtherJobs finish.\r\n");
	return;
}

//直接把分支合并而不经过CA，也不发送邮件
errno_t merge()
{
	//先读取配置文件
	Config_Read();
	
	printf("==================================================\r\n");
	printf("Some Documents will be merge directly...\r\n");

	//CA通过，切换回BRANCH_OFFCIAL分支,并与BRANCH_PUSH分支合并
	system(Str_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));
	system(Str_Cat("cd .. && git merge ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));

	//设置结束状态
	if (ERROR_SUCCESS != status_set("stop"))
	{
		return ERROR_FAILED;
	}

	//system(PATH_SENDMAIL" success");
	printf("Congurations! Merge Successfully! ^_^\r\n");

	return ERROR_SUCCESS;

}

//每次工作前的初始化
void _work_init()
{
	char *pcName = NULL;
	
	//初始化CA结果和编译结果的全局变量
	CA_SetBusy();
	ServerProc_ResetOtherJobsResult();

	//读取配置文件
	Config_Read();

	//每次工作打印开头
	printf("==================================================\r\n");
	printf("Auto CA Start!\r\nPlease do not close the window..\r\n");
	print_time();

	//延时等待远程代码合入BRANCH_PUSH
	Sleep(1000);

	//删除hooks中"wget"产生的临时文件
	pcName = GitAutoCA_GetFileName(FILE_TYPE_WGET_TMP);
	if (0 != strcmp(pcName, FILE_WGET_TMP_DFT))
	{
		system(Str_NCat("del /F ", GitAutoCA_GetFileName(FILE_TYPE_WGET_TMP), NULL));
	}

	return;
}

/***********************************开始工作******************************************
接收到start信号后的工作
*************************************************************************************/
//接收到start信号后的工作（重要）
errno_t work()
{
	char buf[LINE_MAXSIZE] = { 0 };

	//每次工作前的初始化
	_work_init();
	
	//设置状态为CAing
	if (ERROR_SUCCESS != status_set("CAing"))
	{
		return ERROR_FAILED;
	}

	//删除原来的Lint文件
	sprintf_s(buf, sizeof(buf), "del /F %s", GitAutoCA_GetFileName(FILE_TYPE_CA_RESULT));
	system(buf);

	//切换到BRANCH_PUSH分支
	system(Str_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));
	Sleep(500);  //两个分支不同，预留时间切换
	
	//开始CA...
	system(Str_Cat(GitAutoCA_GetFileName(FILE_TYPE_CA_RUN), " >> Run_CA_Log.txt"));

	//等待CA检查完毕
	(void)CA_WaitFinish();

	//切换回OFFICAL分支（发送邮件的脚本要切换到该分支下，且分支未合入OFFICAL）
	system(Str_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));

	//设置CA结果
	(void)CA_JudgeAndSetReulst();
	(void)status_set("CA_finish");

	//等待获取其他任务结果
	_otherjobs_WaitFinish();

	//设置结束状态
	if (ERROR_SUCCESS != status_set("stop"))
	{
		return ERROR_FAILED;
	}	

	printf("Congurations! CA Finished! ^_^\r\n");

	return ERROR_SUCCESS;
}

errno_t _init_get_config()
{
	char buf[LINE_MAXSIZE] = { 0 };
	CONFIG_CFG_S stCfg;
	errno_t err;

	//设置配置文件路径
	err = Config_SetPath(GitAutoCA_GetFileName(FILE_TYPE_CONFIG));
	if (ERROR_SUCCESS != err)
	{
		return err;
	}

	//配置文件的关键字注册

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_TXT_STATUS);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetFileName(FILE_TYPE_STATUS);
	stCfg.size = PATH_FILENAME_MAX;
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_TXT_LINT);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetFileName(FILE_TYPE_CA_RESULT);
	stCfg.size = PATH_FILENAME_MAX;
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_CA_RUN);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetFileName(FILE_TYPE_CA_RUN);
	stCfg.size = PATH_FILENAME_MAX;
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_SENDMAIL);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetFileName(FILE_TYPE_FINI_PY);
	stCfg.size = PATH_FILENAME_MAX;
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_WGET_TMP);
	stCfg.enType = CONFIG_ITEM_TYPE_LINE;
	stCfg.pData = GitAutoCA_GetFileName(FILE_TYPE_WGET_TMP);
	stCfg.size = PATH_FILENAME_MAX;
	err |= Config_Item_Register(&stCfg);

	//----------------------------------------------------------------------

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_CA_PORT);
	stCfg.enType = CONFIG_ITEM_TYPE_NUM;
	stCfg.pData = GitAutoCA_GetCAServerPort_Addr();
	stCfg.size = sizeof(unsigned int);
	err |= Config_Item_Register(&stCfg);

	//----------------------------------------------------------------------

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_BRANCH_OFFCIAL);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL);
	stCfg.size = PATH_BRANCHNAME_MAX;
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_BRANCH_PUSH);
	stCfg.enType = CONFIG_ITEM_TYPE_STRING;
	stCfg.pData = GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH);
	stCfg.size = PATH_BRANCHNAME_MAX;
	err |= Config_Item_Register(&stCfg);

	//----------------------------------------------------------------------
	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_ERROR);
	stCfg.enType = CONFIG_ITEM_TYPE_NUMARRAY;
	stCfg.pData = CA_GetVariAddress(CA_RESULT_TYPE_ERROR);
	stCfg.size = CA_GetVariSize(CA_RESULT_TYPE_ERROR);
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_WARNING);
	stCfg.enType = CONFIG_ITEM_TYPE_NUMARRAY;
	stCfg.pData = CA_GetVariAddress(CA_RESULT_TYPE_WARNING);
	stCfg.size = CA_GetVariSize(CA_RESULT_TYPE_WARNING);
	err |= Config_Item_Register(&stCfg);
	
	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_INFO);
	stCfg.enType = CONFIG_ITEM_TYPE_NUMARRAY;
	stCfg.pData = CA_GetVariAddress(CA_RESULT_TYPE_INFO);
	stCfg.size = CA_GetVariSize(CA_RESULT_TYPE_INFO);
	err |= Config_Item_Register(&stCfg);

	strcpy_s(stCfg.szKeyWord, sizeof(stCfg.szKeyWord), KEYWORD_NOTE);
	stCfg.enType = CONFIG_ITEM_TYPE_NUMARRAY;
	stCfg.pData = CA_GetVariAddress(CA_RESULT_TYPE_NOTE);
	stCfg.size = CA_GetVariSize(CA_RESULT_TYPE_NOTE);
	err |= Config_Item_Register(&stCfg);


	if (ERROR_SUCCESS != err)
	{
		return err;
	}

	//读取配置文件
	err |= Config_Read();

	return err;

}

errno_t Init(int argc, _TCHAR* argv[])
{
	HANDLE handle;
	char szTmp[LINE_MAXSIZE] = { 0 };
	errno_t err = ERROR_SUCCESS;
	
	//获取当前路径,并设置为当前工作目录
	Str_Wchar2Char(argv[0], szTmp, sizeof(szTmp));
	Str_GetBasename_Char(szTmp, szTmp, sizeof(szTmp));
	if (szTmp[0] != '\0' && ERROR_SUCCESS != _chdir(szTmp))
	{
		return ERROR_FAILED;
	}

	//当前工作目录存入全局变量
	memcpy(GitAutoCA_GetPath(), szTmp, strlen(szTmp) + 1);
	_getcwd(szTmp, sizeof(szTmp));
	memcpy(GitAutoCA_GetPathAbso(), szTmp, strlen(szTmp) + 1);
	
	//初始化Log文件 （必须在所有printf之前）
#if WriteLogToFile
	if (ERROR_SUCCESS != WriteLog_Init(PATH_WRITE_LOG))
	{
		return ERROR_FAILED;
	}
#endif	

	//读取配置文件
	err |= _init_get_config();
	
	//清屏并打印信息
	system("cls");
	printf("Git CA Auto Check Tool\r\n");
	printf("MyPATH = %s\r\n", GitAutoCA_GetPath());
	printf("MyPATH_Absolute = %s\r\n", GitAutoCA_GetPathAbso());
	print_time();

	//设置状态为stop
	err |= status_set("stop");

	//切换到BRANCH_OFFCIAL分支
	system(Str_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));
	
	//清理原有的上传分支(在这之前可能遇到意外断电的情况)
	system(Str_Cat("cd .. && git branch -D ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));

	//新建上传分支
	system(Str_Cat("cd .. && git checkout -b ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));

	//切换到BRANCH_OFFCIAL分支
	system(Str_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));
	

	//创建socket监听线程
	handle = CreateThread(NULL, 0, Socket_Start, NULL, 0, NULL);

	return err;
}

void Fini()
{
#if WriteLogToFile
	WriteLog_Fini();
#endif
}

/***********************************主函数******************************************
循环读取状态，读取到start则开始work()
***********************************************************************************/
//主函数：每秒读取一次，等待start来则调用work();有人hello则回应fine
int _tmain(int argc, _TCHAR* argv[])
{

	//============重新开始写罗=============
	char status[LINE_MAXSIZE];
	errno_t err = 0;

	//初始化
	err = Init(argc, argv);
	if (ERROR_SUCCESS != err)
	{
		Fini();
		return err;
	}

	while (1)
	{
		memset(status, 0, sizeof(status));

		//读取状态
		if (ERROR_SUCCESS != status_get(status))
		{
			err = ERROR_FAILED;
			break;
		}

		//如果为hello则回应fine
		if (0 == strncmp(status, "hello", strlen("hello")))
		{
			printf("I`m fine,thank you\r\n");
			if (ERROR_SUCCESS != status_set("fine"))
			{
				err = ERROR_FAILED;
				break;
			}
		}
		//若状态为start则启动
		else if (0 == strncmp(status, "start", strlen("start")))
		{
			//开始作业
			if (ERROR_SUCCESS != work())
			{
				err = ERROR_FAILED;
				break;
			}
		}
		//若状态为merge则直接合入BARNCH_OFFCIAL
		else if (0 == strncmp(status, "merge", strlen("merge")))
		{
			merge();
		}

		//延时1秒
		Sleep(1000);

	}

	//如果遇到错误退出则锁定所有分支
	if (ERROR_SUCCESS != err)
	{
		status_set("lock_all");
	}

	Fini();
	return err;
}

/*
typedef errno_t(*GIT_WORK_PF)(void);
#define GIT_HOOK_NAME_MAX			64
typedef struct tagWorks
{
char szName[GIT_HOOK_NAME_MAX];
GIT_WORK_PF pfWork;
}GIT_WORKS_S;

static GIT_WORKS_S g_stWorks[] =
{
{ "hello", IamFine },
{ "start", work },
{ "merge", merge },
};
*/