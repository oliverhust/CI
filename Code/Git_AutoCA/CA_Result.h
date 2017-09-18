#ifndef __CA_Result_h__
#define __CA_Result_h__

typedef enum
{
	CA_RESULT_TYPE_ERROR = 0,
	CA_RESULT_TYPE_WARNING,
	CA_RESULT_TYPE_INFO,
	CA_RESULT_TYPE_NOTE,
	CA_RESULT_TYPE_MAX

}CA_RESULT_TYPE_E;

/* 设置CA正在忙碌标记 */
void CA_SetBusy();

/* 等待CA结束 */
errno_t CA_WaitFinish();

/* 通过CA文件判断CA是否满足要求，保存结果(成功/失败) */
BOOL CA_JudgeAndSetReulst();

/* 根据CA_JudgeAndSetReulst()保存的结果判断CA是否通过(无需再次读取CA_Lint文件) */
errno_t CA_GetCAResult();

/* 获取变量数组的地址  */
void* CA_GetVariAddress(CA_RESULT_TYPE_E enType);

/* 获取变量的大小 */
int CA_GetVariSize(CA_RESULT_TYPE_E enType);


#endif
