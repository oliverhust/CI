#ifndef __ca_filter_pattern_h__
#define __ca_filter_pattern_h__

/* "短行的最小长度" */
#define CA_FILTER_PATTERN_SHORT_MIN		2

/* "短行的最小大度" */
#define CA_FILTER_PATTERN_SHORT_MAX		6

/* 告警ID字符串的规格 */
#define CA_FILTER_MESS_ID_MAX			8

/* 匹配的准备:预编译正则式 */
errno_t CA_Filter_Pattern_Init();

/* 匹配结束，释放内存 */
void CA_Filter_Pattern_Fini();

/* “有效”字符在2~6个及之间则为“短行” */
BOOL CA_Filter_Pattern_IsShortLine(char *pcPatt);

/* 是否全部为空格字符 */
BOOL CA_Filter_Pattern_IsAllSpace(char *pcPatt);

/* 是否可以直接打印而不需处理 CA告警边界，回车 */
BOOL CA_Filter_Pattern_IsPutsDirect(char *pcPatt);

/* 判断是否为Note信息 */
BOOL CA_Filter_Pattern_IsNote(char *pcPatt);

/* 判断是否Note900后一段的各种告警类型 */
BOOL CA_Filter_Pattern_IsAfterMessage(char *pcPatt);

/* 行识别获取信息:文件名 MD5(无则为"") 告警类型，并输出去掉行号的字符串 */
errno_t CA_Filter_Pattern_LineGetAndModify(char *pcContent, CA_FILTER_SEG_GET_S *pstCfg, char *pcOut, int size);

/* 修改最后的统计信息 */
errno_t CA_Filter_Pattern_ModifyNote(STR_PCREMODIFY_PROC_PF pfModify, char *pcContent, char *pcOut, int size);

#endif