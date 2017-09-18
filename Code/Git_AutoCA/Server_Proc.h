#ifndef __server_proc_h__
#define __server_proc_h__

/* 将状态g_OtherJobsResult置为默认值 */
void ServerProc_ResetOtherJobsResult();

/* 获取编译结果 */
errno_t ServerProc_GetOtherJobsResult();

/* 说明已编译完成，并记录编译结果 */
errno_t ServerProc_RecvOtherJobsResult(char *pcInMsg, char *pcSend, int iSendSize);

/* 回应客户端查询CA结果及文件路径 */
errno_t ServerProc_GetCAResultPath(char *pcInMsg, char *pcSend, int iSendSize);

/* 回应客户端查询Fini.py的路径 */
errno_t ServerProc_GetFiniPyPath(char *pcInMsg, char *pcSend, int iSendSize);



#endif