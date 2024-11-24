#ifndef DISPATCH_SERVER_H
#define DISPATCH_SERVER_H

#include "../Base/forward_list.h"

#include <Windows.h>
#pragma warning(disable: 4996)

#define DISPATCH_SERVER_EVENT_NAME "DispatchServer"

#define MAX_SIZE_SERVICE_NAME 16
#define SERVICE_NAME_POSTFIX ".dll"
#define SERVICE_FUNCTION_NAME "Service"

typedef DWORD (WINAPI *LPSERVICE_FUNCTION)(LPVOID lpParam);

typedef struct _LOADED_LIB {
	HANDLE handle;
	LPCSTR name;
	INT refCount;
} LOADED_LIB, *LPLOADED_LIB;

typedef struct _DISPATCH_SERVER {
	LPFORWARD_LIST_NODE connections;
	LPCRITICAL_SECTION cs;
	LPCSTR postfix;
	HANDLE hThread;
} DISPATCH_SERVER, *LPDISPATCH_SERVER;

DWORD WINAPI DispatchServer(LPVOID lpParam);
HANDLE AddServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
HANDLE FindServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
HANDLE FindAndLoadServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
VOID DeleteServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);

#endif // !DISPATCH_SERVER_H