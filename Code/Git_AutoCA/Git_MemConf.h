#ifndef __git_memconf_h__
#define __git_memconf_h__

#define PATH_MAXSIZE 256

#define PATH_FILENAME_MAX 128

#define PATH_BRANCHNAME_MAX 64

enum FILE_TYPE_E
{
	FILE_TYPE_CONFIG = 0,
	FILE_TYPE_STATUS,
	FILE_TYPE_CA_RESULT,
	FILE_TYPE_CA_RUN,
	FILE_TYPE_FINI_PY,
	FILE_TYPE_WGET_TMP,

	FILE_TYPE_MAX
};

enum BRANCH_TYPE_E
{
	BRANCH_TYPE_OFFCIAL = 0,
	BRANCH_TYPE_PUSH,

	BRANCH_TYPE_MAX
};

char *GitAutoCA_GetPath();
char *GitAutoCA_GetPathAbso();
char *GitAutoCA_GetFileName(enum FILE_TYPE_E FileType);
char *GitAutoCA_GetBranchName(enum BRANCH_TYPE_E BranchType);
unsigned int GitAutoCA_GetCAServerPort();
unsigned int *GitAutoCA_GetCAServerPort_Addr();

#endif