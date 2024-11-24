#include "./console_pipe.h"

#include "../Base/forward_list.h"

#include <stdio.h>

#define ADD_COMMAND(cName, cStruct) \
COMMAND cName = { #cName, cStruct }; \
ForwardListPushFront(commandsList, &cName)

DWORD WINAPI ConsolePipe(LPVOID lpParam) {
	LPCONSOLE_PIPE lpConsolePipe = (LPCONSOLE_PIPE)lpParam;

	HANDLE hPipe;
	if ((hPipe = CreateNamedPipeA("\\\\.\\pipe\\ConcurrentServer", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, NULL, NULL, INFINITE, NULL)) == INVALID_HANDLE_VALUE)
		return -1;

	LPFORWARD_LIST_NODE commandsList;
	if ((commandsList = ForwardListCreateNode(NULL)) == NULL) {
		return -1;
	}

	ADD_COMMAND(start, CommandStart);
	ADD_COMMAND(stop, CommandStop);
	ADD_COMMAND(exit, CommandExit);
	ADD_COMMAND(statistics, CommandStatistics);
	ADD_COMMAND(wait, CommandWait);
	ADD_COMMAND(shutdown, CommandShutdown);

	while (ConnectNamedPipe(hPipe, NULL)) {
		printf("[ConsolePipe] Status: Connected admin\n");
		while (TRUE) {
			CHAR buffer[100];
			lpConsolePipe->output = buffer;
			memset(buffer, 0, sizeof(buffer));
			DWORD rl = 0;
			if (!ReadFile(hPipe, buffer, sizeof(buffer), &rl, NULL)) {
				break;
			}

			BOOL isExist = FALSE;
			LPFORWARD_LIST_NODE it = commandsList;
			while ((it = it->Next) != NULL) {
				LPCOMMAND lpCommand = (LPCOMMAND)it->Data;
				if (!strcmp(lpCommand->name, buffer)) {
					memset(buffer, 0, sizeof(buffer));
					if (!lpCommand->func(*lpConsolePipe)) {
						strcat(buffer, "Error of execute command ");
						strcat(buffer, lpCommand->name);
						strcat(buffer, "\n");
					}

					isExist = TRUE;
					break;
				}
			}

			if (!isExist) {
				memset(buffer, 0, sizeof(buffer));
				strcat(buffer, "Command not exist\n");
			}

			strcat(buffer, "\nSuccess\n");
			DWORD write_size = 0;
			WriteFile(hPipe, buffer, sizeof(buffer), &write_size, NULL);
		}

		printf("[ConsolePipe] Status: Admin connection is lost\n");
		DisconnectNamedPipe(hPipe);
	}

	CloseHandle(hPipe);

	return 0;
}

BOOL CommandStart(CONSOLE_PIPE cp) {
	ResumeThread(cp.as.hThread);
	return TRUE;
}

BOOL CommandStop(CONSOLE_PIPE cp) {
	SuspendThread(cp.as.hThread);
	return TRUE;
}

BOOL CommandExit(CONSOLE_PIPE cp) {
	// Memory lost (need fix)
	TerminateThread(cp.as.hThread, NULL);
	TerminateThread(cp.ds.hThread, NULL);
	TerminateThread(cp.hThread, NULL);
	return TRUE;
}

BOOL CommandStatistics(CONSOLE_PIPE cp) {
	strcat(cp.output, "\nConnections list:\n");

	LPFORWARD_LIST_NODE it = cp.ds.connections;
	INT i = 0;
	while ((it = it->Next) != NULL) {
		LPCONNECTION lpConnection = (LPCONNECTION)it->Data;
		i++;

		CHAR buffer[10];
		strcat(cp.output, itoa(i, buffer, sizeof(buffer)));
		strcat(cp.output, ": Connection ip: ");
		strcat(cp.output, inet_ntoa(lpConnection->addr.sin_addr));
		strcat(cp.output, ":");
		strcat(cp.output, itoa(ntohs(lpConnection->addr.sin_port), buffer, sizeof(buffer)));
		strcat(cp.output, "\n");
	}

	return TRUE;
}

BOOL CommandWait(CONSOLE_PIPE cp) {
	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return FALSE;
	}

	while (cp.ds.connections->Next != NULL) {
		DWORD dwWaitResult;
		while ((dwWaitResult = WaitForSingleObject(hAddConnection, 1)) != WAIT_OBJECT_0);
		Sleep(1000);
	}

	return TRUE;
}

BOOL CommandShutdown(CONSOLE_PIPE cp) {
	CommandWait(cp);
	CommandExit(cp);

	return TRUE;
}