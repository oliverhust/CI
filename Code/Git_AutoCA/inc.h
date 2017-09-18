#ifndef __inc_h__
#define __inc_h__

//用户每次push上传的分支名(默认)
#define BRANCH_PUSH_DFT				" work "
//"官方"分支,正式版本（默认）
#define BRANCH_OFFCIAL_DFT			" master "

//本程序的第一个参数种类
#define ARG_ME1_START				"start"
#define ARG_ME1_MERGE				"merge"

//路径名要和别的脚本约定好(或者从配置文件读取),默认文件名
#define FILE_CONFIG_DFT             "Git_AutoCA_Config.txt"
#define FILE_STATUS_DFT				"sys_status.txt"
#define FILE_CA_RESULT_DFT			"CA_Lint.txt"

//PATH_CA_RUN、PATH_SENDMAIL和本程序要同一目录
#define FILE_CA_RUN_DFT				"Run_CA.bat"
#define FILE_SENDMAIL_DFT			"Fini.py"
#define FILE_WGET_TMP_DFT			"__None__"

//默认socket服务器端口
#define SERVER_PORT_DFT				7777	

//配置文件中的关键字名称
#define KEYWORD_BRANCH_OFFCIAL		"branch_offcial"
#define KEYWORD_BRANCH_PUSH			"branch_push"
#define KEYWORD_ERROR				"error"
#define KEYWORD_WARNING				"warning"
#define KEYWORD_INFO				"info"
#define KEYWORD_NOTE				"note"
#define KEYWORD_TXT_STATUS			"status_txt"
#define KEYWORD_TXT_LINT			"ca_lint_txt"
#define KEYWORD_CA_RUN				"ca_run"
#define KEYWORD_SENDMAIL			"sendmail"
#define KEYWORD_WGET_TMP			"wget_tmp"
#define KEYWORD_CA_PORT				"ca_port"

#define PATH_WRITE_LOG				"git_log_print.txt"

#define LINE_MAXSIZE				512
#define ERROR_FAILED				1		//致命错误，将结束程序
#define ERROR_BUSYING				2		//忙碌中，如编译服务器正在编译...
//#define ERROR_NOT_FOUND			2		//找不到数据
#define GRADE_LINT					20		//Line中有多种error，这个值要大于种类数
#define BUILD_ID_UNKONWN			"0"


#endif