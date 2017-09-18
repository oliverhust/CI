#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "inc.h"
#include "CA_Result.h"
#include "Git_MemConf.h"
#include "Server_Socket.h"
#include "Server_Proc.h"

static char *g_pcSuccessFailed[] = { MSG_SUCCESS, MSG_FAILED, MSG_BUSYING };

/* 其他任务结果的完成与否 ERROR_SUCCESS表示完成 ERROR_BUSY表示正在进行 */
static errno_t g_OtherJobsResult = ERROR_BUSYING;

/* 将状态g_OtherJobsResult置为默认值 */
void ServerProc_ResetOtherJobsResult()
{
	g_OtherJobsResult = ERROR_BUSYING;
	return;
}
/* 获取编译结果 */
errno_t ServerProc_GetOtherJobsResult()
{
	return g_OtherJobsResult;
}

/* 说明已编译完成，并记录编译结果 */
errno_t ServerProc_RecvOtherJobsResult(char *pcInMsg, char *pcSend, int iSendSize)
{
	printf("Receive the message of OtherJobs Finished\r\n");
	
	g_OtherJobsResult = ERROR_SUCCESS;
	
	//通知客户端已成功获得编译结果
	strcpy_s(pcSend, iSendSize, SEND_HEAD_SUCCESS);

	return ERROR_SUCCESS;
}

/* 回应客户端查询CA结果及文件路径 */
errno_t ServerProc_GetCAResultPath(char *pcInMsg, char *pcSend, int iSendSize)
{
	//获取CA结果
	if (ERROR_SUCCESS == CA_GetCAResult())
	{
		//CA路径
		sprintf_s(pcSend, iSendSize, "%s%s\\%s",
			SEND_HEAD_SUCCESS,
			GitAutoCA_GetPathAbso(),
			GitAutoCA_GetFileName(FILE_TYPE_CA_RESULT));
	}
	else if (ERROR_FAILED == CA_GetCAResult())
	{
		//CA路径
		sprintf_s(pcSend, iSendSize, "%s%s\\%s",
			SEND_HEAD_FAILED,
			GitAutoCA_GetPathAbso(),
			GitAutoCA_GetFileName(FILE_TYPE_CA_RESULT));
	}
	else
	{
		strcpy_s(pcSend, iSendSize, SEND_HEAD_BUSY);
	}

	return ERROR_SUCCESS;
}

/* 回应客户端查询Fini.py的路径 */
errno_t ServerProc_GetFiniPyPath(char *pcInMsg, char *pcSend, int iSendSize)
{
	sprintf_s(pcSend, iSendSize, "%s\\%s %s %s %s", 
		GitAutoCA_GetPathAbso(),  GitAutoCA_GetFileName(FILE_TYPE_FINI_PY),
		GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL),
		GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH),
		g_pcSuccessFailed[CA_GetCAResult()]
		);

	return ERROR_SUCCESS;
}

