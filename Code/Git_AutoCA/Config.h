#ifndef __config_h__
#define __config_h__

/* 一行的最长读入长度 含'\0' */
#define CONFIG_LINE_MAXSIZE 512

#define CONFIG_KEYWORD_LEN_MAX 63

/* 最多支持多少的项目  */
#define CONFIG_ITEMS_MAX     256

/* 项目的类型 */
enum CONFIG_ITEM_TYPE_E
{
	CONFIG_ITEM_TYPE_NUM = 0,   /* 单个数值 */
	CONFIG_ITEM_TYPE_NUMARRAY,  /* 数组 */
	CONFIG_ITEM_TYPE_CHAR,      /* 单个字符 */
	CONFIG_ITEM_TYPE_STRING,   /* 连续字符串，不能有空格和其他特殊字符除'_' */
	CONFIG_ITEM_TYPE_LINE,     /* '=' 号后面完整的一行，包含空格 （自动去掉'\n'） */

	CONFIG_ITEM_TYPE_MAX
};

typedef struct tag_CONFIG_CFG_S
{
	char szKeyWord[CONFIG_KEYWORD_LEN_MAX + 1];
	enum CONFIG_ITEM_TYPE_E enType;
	void *pData;    /*  要存储到的变量指针  */
	int size;       /* 变量/变量数组的总字节数 */

}CONFIG_CFG_S;


errno_t Config_SetPath(char *pcPath);
errno_t Config_Item_Register(CONFIG_CFG_S *pstCfg);
errno_t Config_Item_UnRegister(char *pcKeyWord);
errno_t Config_Item_UnRegisterAll();
errno_t Config_PhaseEachLine(char *line);
errno_t Config_Read();

/* 如果注册了多个Key一样的项，则它们都会注册，但使用最新注册的项；
去注册时若有多项相同的Key则总是去注册最新的一项  */


#endif

/*
读取配置文件,如
name = INFO
error = 12  19  5  6
*/
