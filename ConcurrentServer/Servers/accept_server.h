#ifndef ACCEPT_SERVER_H
#define ACCEPT_SERVER_H

#include "../Base/forward_list.h"
#include "../connection.h"

#define ASERV_STATE_CLOSE 0
#define ASERV_STATE_START 1
#define ASERV_STATE_STOP 2

typedef struct _ACCEPT_SERVER {
	SOCKET s;
	SOCKADDR_IN addr;
	LPFORWARD_LIST_NODE connections;
	LPCRITICAL_SECTION cs;
	HANDLE hThread;
	DWORD state;
} ACCEPT_SERVER, *LPACCEPT_SERVER;

DWORD WINAPI AcceptServer(LPVOID lpParam);
BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count);
LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer);
LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer);

BOOL OpenAcceptServer(LPACCEPT_SERVER lpServer);
BOOL StartAcceptServer(LPACCEPT_SERVER lpServer);
BOOL StopAcceptServer(LPACCEPT_SERVER lpServer);
BOOL CloseAcceptServer(LPACCEPT_SERVER lpServer);

#endif // !ACCEPT_SERVER_H