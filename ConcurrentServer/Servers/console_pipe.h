#ifndef CONSOLE_PIPE_H
#define CONSOLE_PIPE_H

#include "./accept_server.h"
#include "./dispatch_server.h"

#include <Windows.h>

typedef struct _CONSOLE_PIPE {
	ACCEPT_SERVER as;
	DISPATCH_SERVER ds;
	HANDLE hThread;
	LPSTR output;
} CONSOLE_PIPE, *LPCONSOLE_PIPE;

typedef struct _COMMAND {
	LPCSTR name;
	BOOL(*func)(CONSOLE_PIPE);
} COMMAND, *LPCOMMAND;

DWORD WINAPI ConsolePipe(LPVOID lpParam);
BOOL CommandStart(CONSOLE_PIPE cp);
BOOL CommandStop(CONSOLE_PIPE cp);
BOOL CommandExit(CONSOLE_PIPE cp);
BOOL CommandStatistics(CONSOLE_PIPE cp);
BOOL CommandWait(CONSOLE_PIPE cp);
BOOL CommandShutdown(CONSOLE_PIPE cp);

#endif // !CONSOLE_PIPE_H