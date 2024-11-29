#include "./main.h"

#include "./Base/forward_list.h"
#include "./Servers/accept_server.h"
#include "./Servers/dispatch_server.h"
#include "./Servers/console_pipe.h"
#include "./Servers/index_server.h"

#define MESSAGE_SIZE 50
#define SERVER_NAME_SIZE 15

HANDLE hAddConnection;

HANDLE hAcceptServer;
HANDLE hDispatchServer;
HANDLE hConsolePipe;
HANDLE hIndexServer;

LPFORWARD_LIST_NODE lpConnections;

VOID FreeConnection(LPVOID lpParam) {
	LPCONNECTION lpConnection = (LPCONNECTION)lpParam;
	if (lpConnection == NULL)
		return;

	closesocket(lpConnection->s);
	HeapFree(GetProcessHeap(), 0, lpConnection);
}

int main(int argc, char* argv[]) {
	printf("[Main] Status: Start Server\n");
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);

	lpConnections = ForwardListCreateNode(NULL);

	if ((hAddConnection = CreateEventA(NULL, TRUE, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
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
	asConfig.hThread = hAcceptServer;

	LIB_LOADER libLoader = { ForwardListCreateNode(NULL)};
	DISPATCH_SERVER dsConfig;
	dsConfig.connections = asConfig.connections;
	dsConfig.lpLibLoader = &libLoader;
	dsConfig.cs = &cs;
	if ((hDispatchServer = CreateThread(NULL, NULL, DispatchServer, &dsConfig, NULL, NULL)) == NULL) {
		return -1;
	}
	dsConfig.hThread = hDispatchServer;

	CONSOLE_PIPE cpConfig;
	cpConfig.as = asConfig;
	cpConfig.ds = dsConfig;
	cpConfig.output = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHAR) * CONSOLE_PIPE_OUT_SIZE);
	memset(cpConfig.output, 0, 512);
	if ((hConsolePipe = CreateThread(NULL, NULL, ConsolePipe, &cpConfig, NULL, NULL)) == NULL) {
		return -1;
	}
	cpConfig.hThread = hConsolePipe;

	Sleep(1000);
	hIndexServer = CreateThread(NULL, NULL, IndexServer, NULL, NULL, NULL);

	WaitForSingleObject(hConsolePipe, INFINITE);
	CloseHandle(hConsolePipe);
	HeapFree(GetProcessHeap(), 0, cpConfig.output);
	WaitForSingleObject(hDispatchServer, INFINITE);
	CloseHandle(hDispatchServer);
	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);

	TerminateThread(hIndexServer, NULL);

	ForwardListFree(lpConnections, FreeConnection);
	DeleteCriticalSection(&cs);
	CloseHandle(hAddConnection);
	
	return 0;
}