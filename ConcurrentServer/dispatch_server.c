#include "./dispatch_server.h"

#include "./accept_server.h"

DWORD WINAPI DispatchServer(LPVOID lpParam) {
	LPDISPATCH_SERVER server = lpParam;
	LPFORWARD_LIST_NODE connections = server->connectios;
	HANDLE hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, "AddConnection");

	LPFORWARD_LIST_NODE libList = NULL;

	while (TRUE) {
		DWORD dwWaitResult;
		while (dwWaitResult != WAIT_OBJECT_0) {
			dwWaitResult = WaitForSingleObject(hAddConnection, 1);
		}

		LPFORWARD_LIST_NODE it;
		while ((it = connections->Next) != NULL) {
			LPCONNECTION connection = it->Data;
			if (strlen(connection->sName) == 0) {
				LPSTR buffer[100];
				INT lenght = 0;
				if ((lenght = recv(connection->s, buffer, sizeof(buffer), NULL)) == SOCKET_ERROR) {
					return -1;
				}

				LPSTR sName = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lenght + 1);
				strcpy(sName, buffer);
				connection->sName = sName;

				HANDLE hService = FindServiceLib(libList, "name");
				if (hService == NULL) {
					AddServiceLib(libList, "name");
				}

				LPSERVICE_FUNCTION lpFunction = GetProcAddress(hService, "functionname");
				if (lpFunction != NULL) {
					CreateThread(NULL, NULL, lpFunction, NULL, NULL, NULL);
				}
			}
		}
	}

	return 0;
}

BOOL AddServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPLOADED_LIB lpLib = (LPLOADED_LIB)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOADED_LIB));
	lpLib->name = name;
	if ((lpLib->handle = LoadLibraryA(name)) == NULL) {
		HeapFree(GetProcessHeap(), 0, lpLib);
		return FALSE;
	}

	ForwardListPushBack(lpLibList, lpLib);
	return TRUE;
}

HANDLE FindServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPFORWARD_LIST_NODE it;
	while ((it = lpLibList->Next) != NULL) {
		LPLOADED_LIB lib = it->Data;
		
		if (strcmp(lib->name, name))
			return lib->handle;
	}

	return NULL;
}

VOID DeleteServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name) {
	LPFORWARD_LIST_NODE it;
	while ((it = lpLibList->Next) != NULL) {
		LPLOADED_LIB lib = it->Data;

		if (strcmp(lib->name, name)) {
			if (--lib->refCount == 0) {
				FreeLibrary(lib->handle);
				ForwardListDeleteNode(lpLibList, lib, ...);
			}

			return;
		}
	}
}