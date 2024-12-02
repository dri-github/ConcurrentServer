#include "./main.h"
#include "./Parsers/ini_parser.h"

#include "./Base/forward_list.h"
#include "./Servers/accept_server.h"
#include "./Servers/DispatchServer/dispatch_server.h"
#include "./Servers/ConsolePipe/console_pipe.h"
#include "./Servers/index_server.h"

#define MESSAGE_SIZE 50
#define SERVER_NAME_SIZE 15

HANDLE hAddConnection;

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

typedef struct _CONFIG_CONTAINER {
	LPFORWARD_LIST_NODE config;
	LPCRITICAL_SECTION cs;
	LPVOID data;
} CONFIG_CONTAINER, *LPCONFIG_CONTAINER;

VOID WordLibLoad(LPCSTR word, LPVOID lpParam) {
	LibLoaderSetState((LPLIB_LOADER)lpParam, word, LIB_STATE_FIXED);
}

VOID WordAcceptServer(LPCSTR word, LPVOID lpParam) {
	LPCONFIG_CONTAINER lpCC = lpParam;
	LPFORWARD_LIST_NODE lpAcceptServers = lpCC->data;

	LPACCEPT_SERVER lpServer;
	if ((lpServer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACCEPT_SERVER))) == NULL) {
		return;
	}

	SOCKADDR_IN addr;

	// Default values
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = INADDR_ANY;

	printf("-> Find word %s\n", word);
	LPFORWARD_LIST_NODE itKeysAServer = find_section(lpCC->config, word)->keyValues;
	while ((itKeysAServer = itKeysAServer->Next) != NULL) {
		LPKEY_VALUE lpKeyAServer = itKeysAServer->Data;
		LPCSTR key = lpKeyAServer->key;
		LPCSTR value = lpKeyAServer->value;

		if (!strcmp(key, "port")) {
			addr.sin_port = htons(atoi(value));
		}
		else if (!strcmp(key, "ip")) {
			addr.sin_addr.s_addr = inet_addr(value);
		}
	}

	lpServer->addr = addr;
	lpServer->connections = lpConnections;
	lpServer->cs = lpCC->cs;

	if (!OpenAcceptServer(lpServer)) {
		HeapFree(GetProcessHeap(), 0, lpServer);
	}
	else {
		ForwardListPushFront(lpAcceptServers, lpServer);
	}
}

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return -1;

	LPFORWARD_LIST_NODE config = ForwardListCreateNode(NULL);
	if (ini_parse("conf.ini", config) < 0) {
		printf("[Main] Error: Read conf.ini file error\n");
		system("pause");
		return -1;
	}

	printf("[Main] Status: Start Server\n");
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);

	lpConnections = ForwardListCreateNode(NULL);

	if ((hAddConnection = CreateEventA(NULL, TRUE, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return -1;
	}

	LPFORWARD_LIST_NODE lpAcceptServers = ForwardListCreateNode(NULL);

	LIB_LOADER libLoader = { ForwardListCreateNode(NULL) };

	SOCKADDR_IN addr;

	// Default values
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = INADDR_ANY;

	LPFORWARD_LIST_NODE itKeys = find_section(config, "main")->keyValues;
	while ((itKeys = itKeys->Next) != NULL) {
		LPKEY_VALUE lpKey = itKeys->Data;
		if (!strcmp(lpKey->key, "accept_servers")) {
			CONFIG_CONTAINER cc;
			cc.config = config;
			cc.cs = &cs;
			cc.data = lpAcceptServers;

			word_trim_parse(lpKey->value, ',', WordAcceptServer, &cc);
		}
		else if (!strcmp(lpKey->key, "startup")) {
			word_trim_parse(lpKey->value, ',', WordLibLoad, &libLoader);
		}
	}

	DISPATCH_SERVER dsConfig;
	dsConfig.connections = lpConnections;
	dsConfig.lpLibLoader = &libLoader;
	dsConfig.cs = &cs;
	if ((hDispatchServer = CreateThread(NULL, NULL, DispatchServer, &dsConfig, NULL, NULL)) == NULL) {
		return -1;
	}
	dsConfig.hThread = hDispatchServer;

	CONSOLE_PIPE cpConfig;
	cpConfig.ass = lpAcceptServers;
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

	TerminateThread(hIndexServer, NULL);

	ForwardListFree(lpConnections, FreeConnection);
	DeleteCriticalSection(&cs);
	CloseHandle(hAddConnection);

	HeapFree(GetProcessHeap(), 0, config);

	if (WSACleanup() == SOCKET_ERROR)
		return -1;

	return 0;
}