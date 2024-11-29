#ifndef CONSOLE_PIPE_H
#define CONSOLE_PIPE_H

#include "../accept_server.h"
#include "../DispatchServer/dispatch_server.h"

#include <Windows.h>

typedef struct _CONSOLE_PIPE {
	ACCEPT_SERVER as;
	DISPATCH_SERVER ds;
	HANDLE hThread;
	LPSTR output;
} CONSOLE_PIPE, *LPCONSOLE_PIPE;

typedef struct _COMMAND {
	LPCSTR name;
	BOOL(*func)(CONSOLE_PIPE, LPCSTR);
} COMMAND, *LPCOMMAND;

DWORD WINAPI ConsolePipe(LPVOID lpParam);
BOOL CommandStart(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandStop(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandExit(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandStatistics(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandWait(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandShutdown(CONSOLE_PIPE cp, LPCSTR argv);
BOOL CommandLibset(CONSOLE_PIPE cp, LPCSTR argv);

//char* trimWord(const char* text) {
//	size_t length = strlen(text);
//
//	char* word = text;
//	for (word = text; word < word + length; word++) {
//		if (*word == ' ') {
//			*(word++) = '\0';
//			break;
//		}
//	}
//
//	return word;
//}

#endif // !CONSOLE_PIPE_H