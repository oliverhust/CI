#ifndef __ca_filter_data__h__
#define __ca_filter_data__h__


/* 最多可以存储的MD5项目数量 */
#define CA_FILTER_DATA_MD5_MAX					8192
/* 最多可以存放可忽略文件名的数量 */
#define CA_FILTER_DATA_FILENAME_MAX				128

typedef struct tagCA_FILTER_MD5
{
	char szMD5[CA_FILTER_MD5_STR_MAX];
}CA_FILTER_MD5_S;

//================================================================================

/* 添加单条CA告警条目 MD5 */
errno_t CA_Filter_Data_Add_MD5(CA_FILTER_MD5_S *pstMD5);

/* 删除单条CA告警条目 MD5 */
errno_t CA_Filter_Data_Del_MD5(CA_FILTER_MD5_S *pstMD5);

/* 删除所有CA告警条目 MD5 */
void CA_Filter_Data_Del_MD5_All();

/* 查询单条CA告警条目 （MD5）是否存在 */
errno_t CA_Filter_Data_IsExist_MD5(CA_FILTER_MD5_S *pstMD5);

/* 计算当前内存中保存的 MD5条目 总数 */
int CA_Filter_Data_Count_MD5();

/* 获取内存中第index个MD5条目的内容 */
errno_t CA_Filter_Data_Get_MD5(int index, CA_FILTER_MD5_S *pstCfg);

//================================================================================

/* 添加单条CA告警条目 文件名 */
errno_t CA_Filter_Data_Add_FileName(char *pcFileName);

/* 删除单条CA告警条目 文件名 */
errno_t CA_Filter_Data_Del_FileName(char *pcFileName);

/* 删除所有CA告警条目 文件名 */
void CA_Filter_Data_Del_FileName_All();

/* 查询单条CA告警条目 （文件名）是否存在 */
errno_t CA_Filter_Data_IsExist_FileName(char *pcFileName);

/* 计算当前内存中保存的 文件名条目 总数 */
int CA_Filter_Data_Count_FileName();

/* 获取内存中第index个 文件名条目 的内容 */
errno_t CA_Filter_Data_Get_FileName(int index, char pcFileName[CA_FILTER_FILENAME_MAX]);

//================================================================================

#endif