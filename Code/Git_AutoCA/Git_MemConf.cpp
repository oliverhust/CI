#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "inc.h"
#include "Git_MemConf.h"

/* 将读到的配置存都存到这个文件的全局变量中，通过函数接口查询 */

/* 当前工作路径（由main函数的argv[0]获得，可能是相对路径也可能是绝对路径）*/
static char MyPath[PATH_MAXSIZE] = { 0 };
/* 当前工作绝对路径 */
static char MyPathAbso[PATH_MAXSIZE] = { 0 };
/* 数组的顺序要和FILE_TYPE_E定义一致 */
static char MyFileName[FILE_TYPE_MAX][PATH_FILENAME_MAX] =
{
	FILE_CONFIG_DFT,
	FILE_STATUS_DFT,
	FILE_CA_RESULT_DFT,
	FILE_CA_RUN_DFT,
	FILE_SENDMAIL_DFT,
	FILE_WGET_TMP_DFT,


};
/* 分支定义的顺序要和BRANCH_TYPE_E一致 */
static char MyBranchName[BRANCH_TYPE_MAX][PATH_BRANCHNAME_MAX] =
{
	" master ",
	" work "
};

/* 监听的全局端口号 */
static unsigned int g_CAServerPort = SERVER_PORT_DFT;

char *GitAutoCA_GetPath()
{
	char *pcRet = MyPath;

	if (NULL == pcRet)
	{
		printf("Get My Path error.\r\n");
	}

	return MyPath;
}

char *GitAutoCA_GetPathAbso()
{
	char *pcRet = MyPathAbso;

	if (NULL == pcRet)
	{
		printf("Get My PathAbsolute error.\r\n");
	}

	return MyPathAbso;
}

char *GitAutoCA_GetFileName(enum FILE_TYPE_E FileType)
{
	char *pcRet = MyFileName[FileType];

	if (NULL == pcRet)
	{
		printf("Get FileName error, file type = %d.\r\n", FileType);
	}

	return MyFileName[FileType];
}

char *GitAutoCA_GetBranchName(enum BRANCH_TYPE_E BranchType)
{
	char *pcRet = MyBranchName[BranchType];

	if (NULL == pcRet)
	{
		printf("Get Branch name error,branch type = %d.\r\n", BranchType);
	}

	return MyBranchName[BranchType];
}

unsigned int GitAutoCA_GetCAServerPort()
{
	return g_CAServerPort;
}

unsigned int *GitAutoCA_GetCAServerPort_Addr()
{
	return &g_CAServerPort;
}