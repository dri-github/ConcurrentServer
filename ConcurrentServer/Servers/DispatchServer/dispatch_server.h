#ifndef DISPATCH_SERVER_H
#define DISPATCH_SERVER_H

#include "../../Base/forward_list.h"
#include "./lib_loader.h"

#include <Windows.h>
#pragma warning(disable: 4996)

#define DISPATCH_SERVER_EVENT_NAME "DispatchServer"

#define MAX_SIZE_SERVICE_NAME 16
#define SERVICE_PATH_PREFIX "./Services/"
#define SERVICE_NAME_POSTFIX ".dll"
#define SERVICE_FUNCTION_NAME "Service"
#define CONSOLE_PIPE_OUT_SIZE 2048

typedef DWORD (WINAPI *LPSERVICE_FUNCTION)(LPVOID lpParam);

typedef struct _DISPATCH_SERVER {
	LPFORWARD_LIST_NODE connections;
    LPLIB_LOADER lpLibLoader;
	LPCRITICAL_SECTION cs;
	LPCSTR postfix;
	HANDLE hThread;
} DISPATCH_SERVER, *LPDISPATCH_SERVER;

DWORD WINAPI DispatchServer(LPVOID lpParam);

#endif // !DISPATCH_SERVER_H