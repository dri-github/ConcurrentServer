#ifndef ACCEPT_SERVER_H
#define ACCEPT_SERVER_H

#include "./main.h"
#include "./forward_list.h"

#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable: 4996)

typedef struct _ACCEPT_SERVER {
	SOCKET s;
	SOCKADDR_IN addr;
	LPFORWARD_LIST_NODE connections;
	LPCRITICAL_SECTION cs;
} ACCEPT_SERVER, *LPACCEPT_SERVER;

#define CONNECTION_STATE_ACCEPTED 0
#define CONNECTION_STATE_SUCCESS 1
#define CONNECTION_STATE_DROPED 2
#define CONNECTION_STATE_ERROR 3

typedef struct _CONNECTION {
	SOCKET s;
	SOCKADDR_IN addr;
	DWORD state;
} CONNECTION, *LPCONNECTION;

DWORD WINAPI AcceptServer(LPVOID lpParam);
BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count);
LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer);
LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer);

#endif // !ACCEPT_SERVER_H