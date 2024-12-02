#include "./accept_server.h"

#define DISPATCH_SERVER_EVENT_NAME "DispatchServer"

DWORD WINAPI AcceptServer(LPVOID lpParam) {
	LPACCEPT_SERVER server = lpParam;
	if ((server->s = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
		return -1;

	if (bind(server->s, (LPSOCKADDR)&server->addr, sizeof(server->addr)) == SOCKET_ERROR)
		return -1;

	if (ChangeConnectionsCount(server, SOMAXCONN))
		return -1;

	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return -1;
	}
	
	LPCSTR ip = inet_ntoa(server->addr.sin_addr);
	USHORT port = ntohs(server->addr.sin_port);

	while (server->state != ASERV_STATE_CLOSE) {
		printf("[AcceptServer %s:%i] Status: Wait Connection\n", ip, port);
		LPCONNECTION lpConnection;
		if ((lpConnection = CreateConnection(server)) == NULL) {
			printf("[AcceptServer %s:%i] Error: CreateConnection\n", ip, port);
			continue;
		}

		lpConnection->state = CONNECTION_STATE_ACCEPTED;

		EnterCriticalSection(server->cs);
		if (ForwardListPushFront(server->connections, lpConnection)) {
			printf("[AcceptServer %s:%i] Status: Connected %s:%i\n", ip, port, inet_ntoa(lpConnection->addr.sin_addr), ntohs(lpConnection->addr.sin_port));
		}
		LeaveCriticalSection(server->cs);

		SetEvent(hAddConnection);
	}

	printf("[AcceptServer %s:%i] Status: Close socket\n", ip, port);

	if (closesocket(server->s) == SOCKET_ERROR)
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

BOOL OpenAcceptServer(LPACCEPT_SERVER lpServer) {
	HANDLE hAcceptServer;
	if ((hAcceptServer = CreateThread(NULL, NULL, AcceptServer, lpServer, NULL, NULL)) == NULL) {
		return FALSE;
	}
	lpServer->hThread = hAcceptServer;
	lpServer->state = ASERV_STATE_START;

	return TRUE;
}

BOOL StartAcceptServer(LPACCEPT_SERVER lpServer) {
	BOOL result = ResumeThread(lpServer->hThread);
	lpServer->state = ASERV_STATE_START;

	return result;
}

BOOL StopAcceptServer(LPACCEPT_SERVER lpServer) {
	BOOL result = SuspendThread(lpServer->hThread);
	lpServer->state = ASERV_STATE_STOP;

	return result;
}

BOOL CloseAcceptServer(LPACCEPT_SERVER lpServer) {
	lpServer->state = ASERV_STATE_CLOSE;
	return TRUE;
}