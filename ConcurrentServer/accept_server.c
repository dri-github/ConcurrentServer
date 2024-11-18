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

	while (TRUE) {
		LPCONNECTION lpConnection = CreateConnection(server);
		if (lpConnection == NULL) {
			return -1;
		}

		ForwardListPushBack(server->connections, lpConnection);
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
	LPCONNECTION lpConnection = (LPCONNECTION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CONNECTION));

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