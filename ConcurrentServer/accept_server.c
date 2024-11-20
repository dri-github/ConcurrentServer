#include "./accept_server.h"

DWORD WINAPI AcceptServer(LPVOID lpParam) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return -1;

	LPACCEPT_SERVER server = lpParam;
	if ((server->s = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
		return -1;

	if (bind(server->s, (LPSOCKADDR)&server->addr, sizeof(server->addr)) == SOCKET_ERROR)
		return -1;

	if (ChangeConnectionsCount(server, SOMAXCONN))
		return -1;

	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, "AddConnection")) == NULL) {
		return -1;
	}

	while (TRUE) {
		printf("[AcceptServer] Status: Wait Connection\n");
		LPCONNECTION lpConnection;
		if ((lpConnection = CreateConnection(server)) == NULL) {
			printf("[AcceptServer] Error: CreateConnection\n");
			continue;
		}

		lpConnection->state = 0;

		EnterCriticalSection(server->cs);
		if (ForwardListPushFront(server->connections, lpConnection)) {
			printf("[AcceptServer] Status: Connected %s:%i\n", inet_ntoa(lpConnection->addr.sin_addr), ntohs(lpConnection->addr.sin_port));
		}
		LeaveCriticalSection(server->cs);

		SetEvent(hAddConnection);
	}

	if (closesocket(server->s) == SOCKET_ERROR)
		return -1;

	if (WSACleanup() == SOCKET_ERROR)
		return -1;

	return 0;
}

BOOL ChangeConnectionsCount(LPACCEPT_SERVER lpServer, INT count) {
	return listen(lpServer->s, count) == SOCKET_ERROR;
}

LPCONNECTION CreateConnection(LPACCEPT_SERVER lpServer) {
	LPCONNECTION lpConnection;
	if ((lpConnection = (LPCONNECTION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CONNECTION))) == NULL) {
		return NULL;
	}

	INT lc = sizeof(lpConnection->addr);
	if ((lpConnection->s = accept(lpServer->s, (SOCKADDR*)&lpConnection->addr, &lc)) == INVALID_SOCKET) {
		HeapFree(GetProcessHeap(), 0, lpConnection);
		return NULL;
	}

	return lpConnection;
}

LPFORWARD_LIST_NODE GetConnectionsList(LPACCEPT_SERVER lpServer) {
	return lpServer->connections;
}