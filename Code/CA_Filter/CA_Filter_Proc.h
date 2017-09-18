#ifndef __ca_filter_proc_h__
#define __ca_filter_proc_h__

/* Note信息处理：根据原来的统计信息修改 */
void CA_Filter_Proc_Note(char *pcSeg, FILE *stream);

/* 普通Seg段处理：解析特征（MD5,文件名）查询数据库决定输出 统计告警 */
void CA_Filter_Proc_Seg(char *pcSeg, FILE *stream);

/* 输入一个段，学习这个段，无法识别则返回ERROR_FAILED */
errno_t CA_Filter_Proc_Study(char *pcSeg);

/* 读取一个文件，文件每行均为要忽略CA告警的代码文件，存入数据库 */
void CA_Filter_Proc_IgnoreFile(FILE *fil);

/* 保存当前内容中的所有配置到流中 */
errno_t CA_Filter_Proc_SaveCfgFile(FILE *fil);

#endif