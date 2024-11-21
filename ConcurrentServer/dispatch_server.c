#include "./dispatch_server.h"

#include "./accept_server.h"

DWORD WINAPI DispatchServer(LPVOID lpParam) {
	LPDISPATCH_SERVER server = lpParam;
	LPFORWARD_LIST_NODE connections = server->connections;

	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return -1;
	}

	LPFORWARD_LIST_NODE libList = ForwardListCreateNode(NULL);

	while (TRUE) {
		DWORD dwWaitResult;
		while ((dwWaitResult = WaitForSingleObject(hAddConnection, 1)) != WAIT_OBJECT_0);

		printf("[DispatchServer] Status: Updating\n");
		EnterCriticalSection(server->cs);
		LPFORWARD_LIST_NODE it = connections;
		LPFORWARD_LIST_NODE itPrev = it;
		while ((it = it->Next) != NULL) {
			LPCONNECTION connection = it->Data;
			switch (connection->state) {
			case CONNECTION_STATE_ACCEPTED: {
					connection->state = CONNECTION_STATE_SUCCESS;
					printf("[DispatchServer] Status: Changing service for %i\n", ntohs(connection->addr.sin_port));

					LPSTR buffer[MAX_SIZE_SERVICE_NAME + 1];
					memset(buffer, 0, sizeof(buffer));
					INT lenght = 0;

					if ((lenght = recv(connection->s, buffer, sizeof(buffer), NULL)) == SOCKET_ERROR) {
						printf("[DispatchServer] Error: recv");
						break;
					}

					LPSTR sName;
					if ((sName = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lenght + strlen(SERVICE_NAME_POSTFIX) + 1)) == NULL) {
						continue;
					}
					strcpy(sName, buffer);
					strcpy(sName + lenght, SERVICE_NAME_POSTFIX);

					HANDLE hService = FindAndLoadServiceLib(libList, sName);
					if (hService == NULL) {
						CHAR error[] = "[DispatchServer] Warning: Service name not exist\n";
						printf("[DispatchServer] Warning: Service name \"%s\" not exist\n", sName);
						printf("[DispatchServer] Status: Drop connection for %i\n", ntohs(connection->addr.sin_port));
						send(connection->s, error, strlen(error) + 1, NULL);
						closesocket(connection->s);
						connection->state = 1;
						continue;
					}

					LPSERVICE_FUNCTION lpFunction = (LPSERVICE_FUNCTION)GetProcAddress(hService, SERVICE_FUNCTION_NAME);
					if (lpFunction != NULL) {
						send(connection->s, buffer, sizeof(buffer), NULL);
						printf("[DispatchServer] Status: The connection %i select service \"%s\"\n", ntohs(connection->addr.sin_port), sName);
						CreateThread(NULL, NULL, lpFunction, connection, NULL, NULL);
					}
					break;
				}
			case CONNECTION_STATE_DROPED:
				printf("[DispatchServer] Status: Droped connection %i\n", ntohs(connection->addr.sin_port));

				// Need added delete lib function
				closesocket(connection->s);
				itPrev->Next = it->Next;
				HeapFree(GetProcessHeap(), 0, it->Data);
				HeapFree(GetProcessHeap(), 0, it);
				it = itPrev;
				break;
			default:
				break;
			}

			itPrev = it;
		}
		LeaveCriticalSection(server->cs);

		ResetEvent(hAddConnection);
	}

	return 0;
}

HANDLE AddServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPLOADED_LIB lpLib = (LPLOADED_LIB)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOADED_LIB));
	if (lpLib == NULL) {
		return NULL;
	}

	lpLib->name = name;
	if ((lpLib->handle = LoadLibraryA(name)) == NULL) {
		HeapFree(GetProcessHeap(), 0, lpLib);
		return NULL;
	}

	ForwardListPushFront(lpLibList, lpLib);
	return lpLib->handle;
}

HANDLE FindServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPFORWARD_LIST_NODE it;
	while ((it = lpLibList->Next) != NULL) {
		LPLOADED_LIB lib = it->Data;
		
		if (!strcmp(lib->name, name)) {
			lib->refCount++;
			return lib->handle;
		}
	}

	return NULL;
}

HANDLE FindAndLoadServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	HANDLE lib = FindServiceLib(lpLibList, name);
	if (lib == NULL)
		lib = AddServiceLib(lpLibList, name);

	return lib;
}

VOID DeleteServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPFORWARD_LIST_NODE it;
	LPFORWARD_LIST_NODE itPrev = lpLibList;
	while ((it = lpLibList->Next) != NULL) {
		LPLOADED_LIB lib = it->Data;

		if (!strcmp(lib->name, name)) {
			if (--lib->refCount == 0) {
				FreeLibrary(lib->handle);
				HeapFree(GetProcessHeap(), 0, lib->name);

				itPrev->Next = it->Next;
				HeapFree(GetProcessHeap(), 0, it);
			}

			return;
		}

		itPrev = it;
	}
}