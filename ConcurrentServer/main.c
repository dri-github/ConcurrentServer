#include "./main.h"

#include "./forward_list.h"
#include "./accept_server.h"
#include "./dispatch_server.h"

#define MESSAGE_SIZE 50
#define SERVER_NAME_SIZE 15

HANDLE hAddConnection;

HANDLE hAcceptServer;
HANDLE hDispatchServer;

LPFORWARD_LIST_NODE lpConnections;

int main(int argc, char* argv[]) {
	printf("[Main] Status: Start Server\n");
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);

	lpConnections = ForwardListCreateNode(NULL);

	if ((hAddConnection = CreateEventA(NULL, FALSE, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return -1;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = INADDR_ANY;

	ACCEPT_SERVER asConfig;
	asConfig.addr = addr;
	asConfig.connections = lpConnections;
	asConfig.cs = &cs;
	if ((hAcceptServer = CreateThread(NULL, NULL, AcceptServer, &asConfig, NULL, NULL)) == NULL) {
		return -1;
	}

	DISPATCH_SERVER dsConfig;
	dsConfig.connections = asConfig.connections;
	dsConfig.cs = &cs;
	if ((hDispatchServer = CreateThread(NULL, NULL, DispatchServer, &dsConfig, NULL, NULL)) == NULL) {
		return -1;
	}

	WaitForSingleObject(hDispatchServer, INFINITE);
	CloseHandle(hDispatchServer);
	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);

	DeleteCriticalSection(&cs);
	CloseHandle(hAddConnection);
	
	return 0;
}