#ifndef __ca_filter_h__
#define __ca_filter_h__

/* 是否只为生成CA_Filter.exe可执行文件 */
#define CA_FILTER_MAKE_EXE

/* CA脚本名称 */
#define CA_FILTER_BAT_FILE							"CAnalyzer.bat"

/* MCA脚本名称 */
#define CA_FILTER_M_BAT_FILE						"MCAnalyzer.bat"

/* 作为MCA时的程序名 */
#define CA_FILTER_MCA_EXE_NAME						"MCA_Filter.exe"

/* 如果定义了则对短行同等处理（学习/过滤）默认是#define */
#define CA_FILTER_PROC_SHORTLINE

/* CA脚本参数最大长度 包含文件路径 */
#define CA_FILTER_ARG_LEN					512

/* 要忽略的文件名的最大长度 */
#define CA_FILTER_FILENAME_MAX				255

/* CA配置文件中开头注释标识 */
#define CA_FILTER_CFG_COMMENT				"#"

/* CA配置文件中要忽略的文件名的开头标识 */
#define CA_FILTER_CFG_FILENAME				"--"

/* 初始化 */ 
errno_t CA_Filter_Init();

/* 去初始化*/
void CA_Filter_Fini();

/* 根据标准的CA Lint文件进行学习 */
errno_t CA_Filter_Study(char *pcStandardFile);

/* 读取配置文件，存入数据库 */
errno_t CA_Filter_ReadCfgFiles(char *pcFileNameFile);

/* 将数据库中的配置保存到配置文件 */
errno_t CA_Filter_SaveCfgFiles(char *pcFileNameFile);

/* 把CA_Lint文件内容进行过滤，结果存到另一个文件 */
errno_t CA_Filter(char *pcSrcFile, char *pcDstFile);

/* 从内存中删除所有学习结果 */
errno_t CA_Filter_ClearAll();

#endif