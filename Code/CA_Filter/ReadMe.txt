static char *pcContent = \
"*** E:\\SVTI\\Source\\NETFWD\\src\\sbin\\tunnel\\xmluiplugin\\tunnel_xml_tunnels.c L753 Warning 426 _xml_TunnelProcTlvMsg(?, ?, !=0, !=0) \r\n"
"    E:\\SVTI\\Source\\NETFWD\\src\\sbin\\tunnel\\xmluiplugin\\tunnel_xml_tunnels.c L676 Warning 426[_xml_TunnelProcTlvMsg][6FE139CB319B099894F74038C800CB1A]\r\n"
"Call to function 'TLV_CreateReceiveMsg(unsigned int, void *, unsigned short)' violates semantic '2p' \r\n";
static char *pcContent1 = "Hello, world!\r\n";

int _tmain(int argc, char* argv[])
{
	pcre *pstRe;
	pcre_extra* pcEx;
	int aiOvector[30], iErroffset, iRc;
	const char* pcError;
	char *pcPattern = "\\s([C-Zc-z_]:.*) [lL]([0-9][0-9]*) (\\w+) (\\d+)";
	char szSub[LINE_MAXSIZE] = { 0 };

	int i, iOffset = 0;

	/* 编译正则式 */
	pstRe = pcre_compile(pcPattern, PCRE_MULTILINE | PCRE_UTF8, &pcError, &iErroffset, NULL);
	if (NULL == pstRe)
	{
		printf("compile regex expression \'%s\' error.\r\n", pcPattern);
		return ERROR_FAILED;
	}

	/* 对编译结果进行学习 */
	pcEx = pcre_study(pstRe, PCRE_STUDY_JIT_COMPILE, &pcError);

	/* 获取多个匹配结果 */
	/* 正则式匹配,返回匹配的偏移位置 */
	while (0 <= (iRc = pcre_exec(pstRe, pcEx, pcContent, strlen(pcContent), iOffset, 0, aiOvector, sizeof(aiOvector) / sizeof(int))))
	{
		printf("Match Regex Expression:\r\n");
		/* 读取出捕获的子串 */
		for (i = 0; i <= 4; ++i)
		{
			pcre_copy_substring(pcContent, aiOvector, iRc, i, szSub, sizeof(szSub));
			printf("Sub%d=%s\r\n", i, szSub);
		}

		/* 设置偏移，准备下一次匹配 */
		iOffset = aiOvector[1];
		printf("iOffser=%d\r\n", iOffset);

		printf("\r\n");
	}

	pcre_free(pstRe);
	system("pause");
	return 0;
}