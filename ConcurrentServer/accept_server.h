#ifndef ACCEPT_SERVER_H
#define ACCEPT_SERVER_H

#include "./forward_list.h"

#include <Windows.h>
#include <WinSock2.h>

typedef struct _ACCEPT_SERVER {
	SOCKET s;
	SOCKADDR_IN addr;
	LPFORWARD_LIST_NODE connections;
} ACCEPT_SERVER, *LPACCEPT_SERVER;

typedef struct _CONNECTION {
	SOCKET s;
	SOCKADDR_IN addr;
} CONNECTION, *LPCONNECTION;

DWORD WINAPI AcceptServer(LPVOID lpParam);
BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count);
LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer);
LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer);

#endif // !ACCEPT_SERVER_H