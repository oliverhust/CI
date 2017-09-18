#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

#include "inc.h"
#include "CA_Result.h"
#include "Git_MemConf.h"
#include "Server_Socket.h"
#include "Server_Proc.h"

#pragma comment(lib, "ws2_32.lib")

SERVER_FUNC_S g_stServerFunction[] = 
{
	{ ServerProc_RecvOtherJobsResult, SEND_HEAD_ALL_JOBS },
	{ ServerProc_GetCAResultPath, SEND_HEAD_CA_RESULT_PATH},
	{ ServerProc_GetFiniPyPath, SEND_HEAD_FINI_PY_PATH}
};

/* 解读接收消息头，调用对应处理函数 */
errno_t _server_ServiceFunctions(char *pcRecv, char *pcSend, int iSendSize)
{
	int i, n, iHeadLen;

	n = sizeof(g_stServerFunction) / sizeof(SERVER_FUNC_S);
	for (i = 0; i < n; i++)
	{
		iHeadLen = strlen(g_stServerFunction[i].szHead);
		if (0 == strncmp(pcRecv, g_stServerFunction[i].szHead, iHeadLen))
		{
			g_stServerFunction[i].pfProc(pcRecv + iHeadLen, pcSend, iSendSize);
			break;
		}
	}

	if (i == n)
	{
		return ERROR_FAILED;
	}

	return ERROR_SUCCESS;
}

/* 根据消息头判别服务种类然后发回消息 */
errno_t _server_Services(SOCKET ConnFd)
{
	int i;
	errno_t ret = ERROR_SUCCESS;
	char szRecvBuf[SERVER_RECV_BUF_MAX] = { 0 };
	char szSendBuf[SERVER_SEND_BUF_MAX] = { 0 };

	while (1)
	{
		i = recv(ConnFd, szRecvBuf, sizeof(szRecvBuf) - 1, 0);
		if (0 == i)
		{
			//由客户端主动关闭连接
			closesocket(ConnFd);
			ret = ERROR_SUCCESS;
			break;
		}
		else if (0 > i)
		{
			printf("A Client`s message receive error.\r\n");
			closesocket(ConnFd);
			ret = ERROR_FAILED;
			break;
		}
		szRecvBuf[i] = '\0';
		printf("Recv:%s\r\n", szRecvBuf);

		if (ERROR_SUCCESS != _server_ServiceFunctions(szRecvBuf, szSendBuf, sizeof(szSendBuf)))
		{
			printf("A Client`s message has error.\r\n");
			closesocket(ConnFd);
			ret = ERROR_FAILED;
			break;
		}

		i = send(ConnFd, szSendBuf, strlen(szSendBuf), 0);
		printf("Send:%s\r\n", szSendBuf);
		if (i != strlen(szSendBuf))
		{
			printf("Server SendError.\r\n");
			closesocket(ConnFd);
		}

	}

	return ret;
}


/* Socket_Server线程 */
DWORD WINAPI Socket_Start(LPVOID pM)
{
	// 加载socket动态链接库(dll)  
	WORD wVersionRequested;
	WSADATA wsaData;    // 这结构是用于接收Wjndows Socket的结构信息的  
	int err;

	wVersionRequested = MAKEWORD(1, 1);   // 请求1.1版本的WinSock库  

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) 
	{
		return -1;          // 返回值为零的时候是表示成功申请WSAStartup  
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) 
	{
		// 检查这个低字节是不是1，高字节是不是1以确定是否我们所请求的1.1版本  
		// 否则的话，调用WSACleanup()清除信息，结束函数  
		WSACleanup();
		return -1;
	}

	// 创建socket操作，建立流式套接字，返回套接字号sockSrv  
	// SOCKET socket(int af, int type, int protocol);  
	// 第一个参数，指定地址簇(TCP/IP只能是AF_INET，也可写成PF_INET)  
	// 第二个，选择套接字的类型(流式套接字)，第三个，特定地址家族相关协议（0为自动）  
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockSrv)
	{
		printf("Create socket error\r\n");
		return ERROR_FAILED;
	}

	// 套接字sockSrv与本地地址相连  
	// int bind(SOCKET s, const struct sockaddr* name, int namelen);  
	// 第一个参数，指定需要绑定的套接字；  
	// 第二个参数，指定该套接字的本地地址信息，该地址结构会随所用的网络协议的不同而不同  
	// 第三个参数，指定该网络协议地址的长度  
	// PS: struct sockaddr{ u_short sa_family; char sa_data[14];};  
	//                      sa_family指定该地址家族， sa_data起到占位占用一块内存分配区的作用  
	//     在TCP/IP中，可使用sockaddr_in结构替换sockaddr，以方便填写地址信息  
	//   
	//     struct sockaddr_in{ short sin_family; unsigned short sin_port; struct in_addr sin_addr; char sin_zero[8];};  
	//     sin_family表示地址族，对于IP地址，sin_family成员将一直是AF_INET。  
	//     sin_port指定将要分配给套接字的端口。  
	//     sin_addr给出套接字的主机IP地址。  
	//     sin_zero[8]给出填充数，让sockaddr_in与sockaddr结构的长度一样。  
	//     将IP地址指定为INADDR_ANY，允许套接字向任何分配给本地机器的IP地址发送或接收数据。  
	//     如果想只让套接字使用多个IP中的一个地址，可指定实际地址，用inet_addr()函数。  
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 将INADDR_ANY转换为网络字节序，调用 htonl(long型)或htons(整型)  
	//addrSrv.sin_addr.S_un.S_addr = htonl(inet_addr("192.168.56.28"));
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(GitAutoCA_GetCAServerPort());

	if (-1 == bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) // 第二参数要强制类型转换  
	{
		printf("Bind port %d Error\r\n", GitAutoCA_GetCAServerPort());
		return ERROR_FAILED;
	}

	// 将套接字设置为监听模式（连接请求）， listen()通知TCP服务器准备好接收连接  
	// int listen(SOCKET s,  int backlog);  
	// 第一个参数指定需要设置的套接字，第二个参数为（等待连接队列的最大长度）  
	if (-1 == listen(sockSrv, SERVER_LISTEN_MAX))
	{
		printf("Listen %d Nums Error\r\n", SERVER_LISTEN_MAX);
		return ERROR_FAILED;
	}

	// accept()，接收连接，等待客户端连接  
	// SOCKET accept(  SOCKET s,  struct sockaddr* addr,  int* addrlen);  
	// 第一个参数，接收一个处于监听状态下的套接字  
	// 第二个参数，sockaddr用于保存客户端地址的信息  
	// 第三个参数，用于指定这个地址的长度  
	// 返回的是向与这个监听状态下的套接字通信的套接字  

	// 客户端与用户端进行通信  

	// send(), 在套接字上发送数据  
	// int send( SOCKET s,  const char* buf,  int len,  int flags);  
	// 第一个参数，需要发送信息的套接字，  
	// 第二个参数，包含了需要被传送的数据，  
	// 第三个参数是buffer的数据长度，  
	// 第四个参数，一些传送参数的设置  

	// recv(), 在套接字上接收数据  
	// int recv(  SOCKET s,  char* buf,  int len,  int flags);  
	// 第一个参数，建立连接后的套接字，  
	// 第二个参数，接收数据  
	// 第三个参数，接收数据的长度，  
	// 第四个参数，一些传送参数的设置  

	SOCKADDR_IN  addrClient;
	int len = sizeof(SOCKADDR);

	// 不断等待客户端请求的到来
	while(1)
	{      
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
		if (-1 == sockConn)
		{
			printf("Accept a Client Error.\r\n");
			continue;
		}
		
		//根据消息头完成对应服务
		(void)_server_Services(sockConn);
	}

	return ERROR_SUCCESS;
}