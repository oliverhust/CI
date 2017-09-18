#ifndef __server_socket__h__
#define __server_socket__h__

#define SERVER_LISTEN_MAX			10
#define SERVER_RECV_BUF_MAX			8000
#define SERVER_SEND_BUF_MAX			8000

#define SEND_HEAD_LEN_MAX			64
#define SEND_HEAD_SUCCESS			"[Success]"
#define SEND_HEAD_FAILED			"[Failure]"
#define SEND_HEAD_BUSY				"[Busying]"
#define SEND_HEAD_ALL_JOBS			"[All jobs Finished]"
#define SEND_HEAD_CA_RESULT_PATH	"[What is CA Result and Path]"
#define SEND_HEAD_FINI_PY_PATH		"[Give me the Path of Fini_py]"

#define MSG_OTHERJOBS_SUCCESS		"Success"
#define MSG_OTHERJOBS_FAILURE		"Failure"

#define MSG_SUCCESS					"Success"
#define MSG_FAILED					"Failed"
#define MSG_BUSYING					"Busying"

typedef errno_t(*SERVER_PROC_PF)(char *pcInMsg, char *pcSend, int iSendSize);

typedef struct tag_ServerFunc
{
	SERVER_PROC_PF pfProc;
	char szHead[SEND_HEAD_LEN_MAX];
}SERVER_FUNC_S;

/* Socket_Serverœﬂ≥Ã */
DWORD WINAPI Socket_Start(LPVOID pM);

#endif