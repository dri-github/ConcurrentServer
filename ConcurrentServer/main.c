#include "./forward_list.h"
#include "./accept_server.h"
#include "./dispatch_server.h"

#include <Windows.h>
#include <WinSock2.h>

#define MESSAGE_SIZE 50
#define SERVER_NAME_SIZE 15

HANDLE hAcceptServer;
HANDLE hDispatchServer;

int main(int argc, char* argv[]) {
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);

	LPFORWARD_LIST_NODE connections = ForwardListCreateNode(NULL);

	HANDLE hAddConnection = CreateEvent(NULL, FALSE, FALSE, "AddConnection");

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = INADDR_ANY;

	ACCEPT_SERVER asConfig;
	asConfig.addr = addr;
	asConfig.connections = connections;

	hAcceptServer = CreateThread(NULL, NULL, AcceptServer, &asConfig, NULL, NULL);
	hDispatchServer = CreateThread(NULL, NULL, DispatchServer, connections, NULL, NULL);

	WaitForSingleObject(hDispatchServer, INFINITE);
	CloseHandle(hDispatchServer);
	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);

	CloseHandle(hAddConnection);
	
	return 0;
}