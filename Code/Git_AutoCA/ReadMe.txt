========================================================================
    控制台应用程序：Git_AutoCA 项目概述
========================================================================

应用程序向导已为您创建了此 Git_AutoCA 应用程序。

本文件概要介绍组成 Git_AutoCA 应用程序的每个文件的内容。


Git_AutoCA.vcxproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件，其中包含生成该文件的 Visual C++ 的版本信息，以及有关使用应用程序向导选择的平台、配置和项目功能的信息。

Git_AutoCA.vcxproj.filters
    这是使用“应用程序向导”生成的 VC++ 项目筛选器文件。它包含有关项目文件与筛选器之间的关联信息。在 IDE 中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。例如，“.cpp”文件与“源文件”筛选器关联。

Git_AutoCA.cpp
    这是主应用程序源文件。

/////////////////////////////////////////////////////////////////////////////
其他标准文件:

StdAfx.h, StdAfx.cpp
    这些文件用于生成名为 Git_AutoCA.pch 的预编译头 (PCH) 文件和名为 StdAfx.obj 的预编译类型文件。

/////////////////////////////////////////////////////////////////////////////
其他注释:

应用程序向导使用“TODO:”注释来指示应添加或自定义的源代码部分。

/////////////////////////////////////////////////////////////////////////////

//原Git_AutoCA的work处理流程
/*
	//构建成功/失败判断
	if (ERROR_SUCCESS != CA_GetCAResult() || ERROR_SUCCESS != ServerProc_GetOtherJobsResult())
	{
		//构建失败的处理：发送邮件、版本回退

		//切换回OFFICAL分支（发送邮件的脚本要切换到该分支下，且分支未合入OFFICAL）
		system(Mystr_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));

		//调用发送邮件的脚本
		_work_SendEmail(ERROR_FAILED);

		//切换回OFFICAL，删除BRANCH_PUSH，然后重新建立
		system(Mystr_Cat("cd .. && git branch -D ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));
		system(Mystr_Cat("cd .. && git checkout -b ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));

		//切换回BRANCH_OFFCIAL分支
		system(Mystr_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));

		//设置结束状态
		if (ERROR_SUCCESS != status_set("stop"))
		{
			return ERROR_FAILED;
		}
		printf("Could not pass Exam. Merge Failed!\r\n");

		return ERROR_SUCCESS;
	}

	//构建成功的处理：发送邮件、版本合入主线

	//切换回BRANCH_OFFCIAL分支（发送邮件的脚本要切换到该分支下，且分支未合入OFFICAL）
	system(Mystr_Cat("cd .. && git checkout ", GitAutoCA_GetBranchName(BRANCH_TYPE_OFFCIAL)));

	//调用发送邮件的脚本通知用户
	_work_SendEmail(ERROR_SUCCESS);

	//BRANCH_OFFCIAL分支与BRANCH_PUSH分支合并
	system(Mystr_Cat("cd .. && git merge ", GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH)));
*/



/* 发送邮件 参数：ERROR_SUCCESS/ERROR_FAILED 0/1 */
void _work_SendEmail(errno_t err)
{
	char *pcCmd;

	pcCmd = Mystr_NCat(GitAutoCA_GetFileName(FILE_TYPE_FINI_PY), " ",
		GitAutoCA_GetBranchName(BRANCH_TYPE_PUSH), " ",	//参数一：上传分支名
		g_pcSuccessFailed[err],							//参数二：最终结果		
		NULL);
	printf("%s\r\n", pcCmd);
	system(pcCmd);

	return;
}