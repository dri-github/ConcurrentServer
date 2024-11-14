#include <Windows.h>
#include <WinSock2.h>

#define MESSAGE_SIZE 50
#define SERVER_NAME_SIZE 15

HANDLE hAcceptServer;
HANDLE hConsolePipe;
HANDLE hGarbageCleaner;

typedef struct {
	struct LIST* next;
	LPVOID item;
} LIST_ITEM;
typedef LIST_ITEM* LIST;
typedef LIST* LPLIST;

void InitList(LPLIST lpList) {
	LIST list = malloc(sizeof(LIST_ITEM));
	list->next = NULL;
	*lpList = list;
}

void PushList(LPLIST lpList, LPVOID item) {
	LIST current = malloc(sizeof(LIST_ITEM));
	current->item = item;
	current->next = *lpList;
	*lpList = current;
}

void PopList(LPLIST lpList) {
	LIST current = *lpList;
	*lpList = current->next;
	frre(current->item);
	free(current);
}

typedef struct {
	SOCKET s;
	SOCKADDR_IN addr;
	HANDLE hThread;
} CLIENT;
typedef CLIENT* LPCLIENT;

LPCLIENT GetClient(SOCKET sServer) {
	LPCLIENT client = malloc(sizeof(LPCLIENT));
	INT lc = sizeof(client->addr);

	if ((client->s = accept(sServer, (LPSOCKADDR)&client->addr, &lc)) == INVALID_SOCKET)
		return NULL;

	return client;
}

typedef struct {
	SOCKET s;
	SOCKADDR_IN addr;

	LIST clients;
} ACCEPT_SERVER;

DWORD WINAPI EchoServer(LPVOID pPrm) {
	while (TRUE) {

	}

	return 0;
}

DWORD WINAPI AcceptServer(LPVOID pPrm) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return -1;
	
	ACCEPT_SERVER as;
	SOCKET sServer = as.s;
	if ((sServer = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
		return -1;

	if (bind(sServer, (LPSOCKADDR)&as.addr, sizeof(as.addr)) == SOCKET_ERROR)
		return -1;

	if (listen(sServer, SOMAXCONN) == SOCKET_ERROR)
		return -1;

	while (TRUE) {
		LPCLIENT client;
		if ((client = GetClient(sServer)) == NULL)
			break;

		client->hThread = CreateThread(NULL, NULL, EchoServer, client, NULL, NULL);
		PushList(&as.clients, client);
	}

	if (WSACleanup() == SOCKET_ERROR)
		return -1;
	return 0;
}
DWORD WINAPI ConsolePipe(LPVOID pPrm) {
	return 0;
}
DWORD WINAPI GarbageCleaner(LPVOID pPrm) {
	return 0;
}

int main(int argc, char* argv[]) {
	CRITICAL_SECTION critical_section;
	InitializeCriticalSection(&critical_section);

	hAcceptServer = CreateThread(NULL, NULL, AcceptServer, &critical_section, NULL, NULL);
	hConsolePipe = CreateThread(NULL, NULL, ConsolePipe, &critical_section, NULL, NULL);
	hGarbageCleaner = CreateThread(NULL, NULL, GarbageCleaner, &critical_section, NULL, NULL);

	

	DeleteCriticalSection(&critical_section);

	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);
	WaitForSingleObject(hConsolePipe, INFINITE);
	CloseHandle(hConsolePipe);
	WaitForSingleObject(hGarbageCleaner, INFINITE);
	CloseHandle(hGarbageCleaner);

	return 0;
}