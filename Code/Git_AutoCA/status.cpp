#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>
#include <time.h>

#include "inc.h"
#include "status.h"
#include "Git_MemConf.h"
//状态设置，状态有hello,fine,start,CAing,CA_finish,stop,lock_all几种，平常是stop
errno_t status_set(char *status)
{
	FILE *FL;
	errno_t err = ERROR_SUCCESS;

	err = fopen_s(&FL, GitAutoCA_GetFileName(FILE_TYPE_STATUS), "w");
	if (ERROR_SUCCESS != err || NULL == FL)
	{
		printf("Unable to open when set status\r\n");
		return err;
	}

	fputs(status, FL);
	fclose(FL);

	return err;
}

errno_t status_get(char *status)
{
	FILE *FL;
	errno_t err = 0;

	err = fopen_s(&FL, GitAutoCA_GetFileName(FILE_TYPE_STATUS), "r");
	if (ERROR_SUCCESS != err || NULL == FL)
	{
		printf("Unable to open when get status\r\n");
		return err;
	}
	fgets(status, LINE_MAXSIZE, FL);
	fclose(FL);

	return err;
}
