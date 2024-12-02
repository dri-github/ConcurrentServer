#include "./console_pipe.h"

#include "../../Base/forward_list.h"
#include "../../UI/ui.h"
#include "./Commands/libctl/libctl.h"

#include <stdio.h>

#define ADD_COMMAND(cName, cStruct) \
COMMAND cName = { #cName, cStruct }; \
ForwardListPushFront(commandsList, &cName)

DWORD WINAPI ConsolePipe(LPVOID lpParam) {
	LPCONSOLE_PIPE lpConsolePipe = (LPCONSOLE_PIPE)lpParam;

	HANDLE hPipe;
	if ((hPipe = CreateNamedPipeA("\\\\.\\pipe\\ConcurrentServer", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, CONSOLE_PIPE_OUT_SIZE, CONSOLE_PIPE_OUT_SIZE, INFINITE, NULL)) == INVALID_HANDLE_VALUE)
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
	ADD_COMMAND(libctl, CommandLibCtl);

	while (ConnectNamedPipe(hPipe, NULL)) {
		printf("[ConsolePipe] Status: Connected admin\n");
		while (TRUE) {
			CHAR buffer[512];
			memset(buffer, 0, sizeof(buffer));
			DWORD rl = 0;
			if (!ReadFile(hPipe, buffer, sizeof(buffer), &rl, NULL)) {
				break;
			}

			LPSTR secondWord = trimWord(buffer);

			memset(lpConsolePipe->output, 0, CONSOLE_PIPE_OUT_SIZE);

			BOOL isExist = FALSE;
			LPFORWARD_LIST_NODE it = commandsList;
			while ((it = it->Next) != NULL) {
				LPCOMMAND lpCommand = (LPCOMMAND)it->Data;
				if (!strcmp(lpCommand->name, buffer)) {
					if (!lpCommand->func(*lpConsolePipe, secondWord)) {
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

			strcat(lpConsolePipe->output, "\nSuccess\n");
			DWORD write_size = 0;
			WriteFile(hPipe, lpConsolePipe->output, strlen(lpConsolePipe->output) + 1, &write_size, NULL);
		}

		printf("[ConsolePipe] Status: Admin connection is lost\n");
		DisconnectNamedPipe(hPipe);
	}

	CloseHandle(hPipe);

	return 0;
}

BOOL CommandStart(CONSOLE_PIPE cp, LPCSTR argv) {
	ResumeThread(cp.as.hThread);
	return TRUE;
}

BOOL CommandStop(CONSOLE_PIPE cp, LPCSTR argv) {
	SuspendThread(cp.as.hThread);
	return TRUE;
}

BOOL CommandExit(CONSOLE_PIPE cp, LPCSTR argv) {
	// Memory lost (need fix)
	TerminateThread(cp.as.hThread, NULL);
	TerminateThread(cp.ds.hThread, NULL);
	TerminateThread(cp.hThread, NULL);
	return TRUE;
}

BOOL CommandStatistics(CONSOLE_PIPE cp, LPCSTR argv) {
	char_column columns[5] = {
		{ 4, ' ', TA_HORIZONTAL_CENTER },
		{ 20, ' ', TA_HORIZONTAL_CENTER },
		{ 18, ' ', TA_HORIZONTAL_CENTER },
		{ 9, ' ', TA_HORIZONTAL_CENTER },
		{ 21, ' ', TA_HORIZONTAL_CENTER }
	};

	LPCSTR** array = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 20 * sizeof(LPCSTR*));
	for (int i = 0; i < 20; i++) {
		array[i] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 5 * sizeof(LPCSTR));
		for (int j = 0; j < 5; j++) {
			array[i][j] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 30 * sizeof(CHAR));
		}
	}
	
#define SetColumn(id, name) memcpy(array[0][id], name, strlen(name) + 1)
	SetColumn(0, "ID");
	SetColumn(1, "Service");
	SetColumn(2, "IP");
	SetColumn(3, "Port");
	SetColumn(4, "Time");

	char_table table;
	table.columns_count = 5;
	table.columns = columns;
	table.array = array;

	LPFORWARD_LIST_NODE it = cp.ds.connections;
	INT i = 1;
	while ((it = it->Next) != NULL && i < 20) {
		LPCONNECTION lpConnection = (LPCONNECTION)it->Data;
		printf("%i\n", i);

		// ID
		CHAR buffer[64];
		memset(buffer, 0, sizeof(buffer));
		itoa(i, buffer, 10);
		memcpy(array[i][0], buffer, strlen(buffer) + 1);

		// Service Name
		memcpy(array[i][1], lpConnection->sName, strlen(lpConnection->sName) + 1);

		// IP
		CHAR* ipText = inet_ntoa(lpConnection->addr.sin_addr);
		memcpy(array[i][2], ipText, strlen(ipText) + 1);

		// Port
		memset(buffer, 0, sizeof(buffer));
		itoa(ntohs(lpConnection->addr.sin_port), buffer, 10);
		memcpy(array[i][3], buffer, strlen(buffer) + 1);

		// Time
		memset(buffer, 0, sizeof(buffer));
		strftime(buffer, sizeof(buffer), "%d.%m.%y/%H:%M:%S", localtime(&lpConnection->tStart));
		memcpy(array[i][4], buffer, strlen(buffer) + 1);

		i++;
	}
	table.lines_count = i;

	draw_table(table, cp.output);

	for (int i = 0; i < 20; i++) {
		//for (int j = 0; j < 5; j++) {
			//HeapFree(GetProcessHeap(), 0, array[i][j]);
		//}
		HeapFree(GetProcessHeap(), 0, array[i]);
	}
	HeapFree(GetProcessHeap(), 0, array);

	return TRUE;
}

BOOL CommandWait(CONSOLE_PIPE cp, LPCSTR argv) {
	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return FALSE;
	}

	CommandStop(cp, argv);

	while (cp.ds.connections->Next != NULL) {
		DWORD dwWaitResult;
		while ((dwWaitResult = WaitForSingleObject(hAddConnection, 1)) != WAIT_OBJECT_0);
		Sleep(1000);
	}

	CommandStart(cp, argv);

	return TRUE;
}

BOOL CommandShutdown(CONSOLE_PIPE cp, LPCSTR argv) {
	CommandWait(cp, argv);
	CommandExit(cp, argv);

	return TRUE;
}