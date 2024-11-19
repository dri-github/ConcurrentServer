#ifndef DISPATCH_SERVER_H
#define DISPATCH_SERVER_H

#include "./forward_list.h"

#include <Windows.h>

typedef DWORD (*LPSERVICE_FUNCTION)(LPVOID lpParam);

typedef struct _LOADED_LIB {
	HANDLE handle;
	LPCSTR name;
	INT refCount;
} LOADED_LIB, *LPLOADED_LIB;

typedef struct _DISPATCH_SERVER {
	LPFORWARD_LIST_NODE connections;
	LPCRITICAL_SECTION cs;
} DISPATCH_SERVER, *LPDISPATCH_SERVER;

DWORD WINAPI DispatchServer(LPVOID lpParam);
BOOL AddServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
HANDLE FindServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
VOID DeleteServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);

#endif // !DISPATCH_SERVER_H