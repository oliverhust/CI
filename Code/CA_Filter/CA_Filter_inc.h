#ifndef __ca_filter_inc_h__
#define __ca_filter_inc_h__

/* 配置文件的MD5注释头 */
#define CA_FILTER_CFG_COMMENT_MD5_HEAD				"# The MD5 of CA error:"

/* 配置文件的忽略文件名注释头 */
#define CA_FILTER_CFG_COMMENT_FILE_HEAD				"# The Files with CA error to be ignore:"

/* 每一行最大字节数（通用） */
#define LINE_MAXSIZE					1024

/* 最大的段字节数 */
#define SEGMENT_MAXSIZE					8192

/* 尝试打开CA结果原文件的次数 */
#define CA_FILTER_OPENORIGIN_TIMES		20

/* 尝试打开CA结果原文件的时间间隔 毫秒 */
#define CA_FILTER_OPENORIGIN_INTERVAL	200

#define IN
#define OUT
#define ERROR_FAILED					1
#define BOOL_TRUE						1
#define BOOL_FALSE						0

#define CA_FILTER_MD5_STR_MAX			33
#define CA_FILTER_MESSAGE_TYPE_LEN_MAX	16

typedef enum
{
	CA_FILTER_MESSAGE_ERROR = 0,
	CA_FILTER_MESSAGE_WARNING,
	CA_FILTER_MESSAGE_INFO,
	CA_FILTER_MESSAGE_NOTE,
	CA_FILTER_MESSAGE_MAX
}CA_FILTER_MESSAGE_E;

typedef struct tagMessageCount
{
	int iError;
	int iWarning;
	int iInfo;
	int iNote;
}CA_FILTER_MESSAGE_S;

typedef struct tagCA_FILTER_SEG_GET
{
	CA_FILTER_MESSAGE_E enType;
	char szFileName[CA_FILTER_FILENAME_MAX];
	char szMD5[CA_FILTER_MD5_STR_MAX];
}CA_FILTER_SEG_GET_S;

#ifdef CA_FILTER_MAKE_EXE

/* CA原结果临时文件名 */
#define CA_FILTER_ORIGIN_RESULT						"CA_Filter_Result_Origin.txt"

/* CA过滤后的结果文件名 */
#define CA_FILTER_NEW_RESULT						"CA_Filter_Result_New.txt"

/* 每次读取的配置文件名 */
#define CA_FILTER_READ_CFG_FILE						"CA_Filter.cfg"

/* 学习的文件名 */
#define CA_FILTER_STUDY_FILE						"CA_Filter_Study.txt"

/* 自动保存配置的文件名 */
#define CA_FILTER_AUTOSAVE_CFGFILE					"CA_Filter_AutoSave.cfg"

#endif

#endif