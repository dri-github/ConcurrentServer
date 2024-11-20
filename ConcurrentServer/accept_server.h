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

typedef enum _CONNECTION_STATE {
	ACCEPTED = 0,
	ROUTED
} CONNECTION_STATE;

typedef struct _CONNECTION {
	SOCKET s;
	SOCKADDR_IN addr;
	CONNECTION_STATE state;
} CONNECTION, *LPCONNECTION;

DWORD WINAPI AcceptServer(LPVOID lpParam);
BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count);
LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer);
LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer);

#endif // !ACCEPT_SERVER_H