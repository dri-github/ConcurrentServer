#ifndef ACCEPT_SERVER_H
#define ACCEPT_SERVER_H

#include "../Base/forward_list.h"
#include "../connection.h"

typedef struct _ACCEPT_SERVER {
	SOCKET s;
	SOCKADDR_IN addr;
	LPFORWARD_LIST_NODE connections;
	LPCRITICAL_SECTION cs;
} ACCEPT_SERVER, *LPACCEPT_SERVER;

DWORD WINAPI AcceptServer(LPVOID lpParam);
BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count);
LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer);
LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer);

#endif // !ACCEPT_SERVER_H