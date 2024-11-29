#include "./console_pipe.h"

#include "../../Base/forward_list.h"
#include "../../UI/ui.h"

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

	while (ConnectNamedPipe(hPipe, NULL)) {
		printf("[ConsolePipe] Status: Connected admin\n");
		while (TRUE) {
			CHAR buffer[512];
			memset(buffer, 0, sizeof(buffer));
			DWORD rl = 0;
			if (!ReadFile(hPipe, buffer, sizeof(buffer), &rl, NULL)) {
				break;
			}

			LPSTR secondWord;
			for (secondWord = buffer; secondWord < buffer + strlen(buffer); secondWord++) {
				if (*secondWord == ' ') {
					*(secondWord++) = '\0';
					break;
				}
			}

			BOOL isExist = FALSE;
			LPFORWARD_LIST_NODE it = commandsList;
			while ((it = it->Next) != NULL) {
				LPCOMMAND lpCommand = (LPCOMMAND)it->Data;
				if (!strcmp(lpCommand->name, buffer)) {
					memset(buffer, 0, sizeof(buffer));
					memset(lpConsolePipe->output, 0, CONSOLE_PIPE_OUT_SIZE);
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
	strcat(cp.output, "\nConnections list:\n");

	SIZE_T iLength = 4;
	SIZE_T sNameLength = 20;
	SIZE_T ipLength = 21;
	SIZE_T portLength = 8;
	SIZE_T timeLength = 21;

	CHAR buf[512] = "ID";
	strcat(cp.output, horizontal_align_text(buf, iLength, ' ', TA_HORIZONTAL_CENTER));
	memcpy(buf, "Service", strlen("Service") + 1);
	strcat(cp.output, horizontal_align_text(buf, sNameLength, ' ', TA_HORIZONTAL_CENTER));
	memcpy(buf, "IP", strlen("IP") + 1);
	strcat(cp.output, horizontal_align_text(buf, ipLength, ' ', TA_HORIZONTAL_CENTER));
	memcpy(buf, "Port", strlen("Port") + 1);
	strcat(cp.output, horizontal_align_text(buf, portLength, ' ', TA_HORIZONTAL_CENTER));
	memcpy(buf, "Connect Time", strlen("Connect Time") + 1);
	strcat(cp.output, horizontal_align_text(buf, timeLength, ' ', TA_HORIZONTAL_CENTER));

	strcat(cp.output, "\n");
	memset(buf, 205, iLength + sNameLength + ipLength + portLength + timeLength);
	strcat(cp.output, buf);
	strcat(cp.output, "\n");

	LPFORWARD_LIST_NODE it = cp.ds.connections;
	INT i = 0;
	while ((it = it->Next) != NULL) {
		LPCONNECTION lpConnection = (LPCONNECTION)it->Data;
		i++;

		CHAR buffer[512];
		memset(buffer, 0, sizeof(buffer));
		strcat(cp.output, horizontal_align_text(itoa(i, buffer, 10), iLength, ' ', TA_HORIZONTAL_CENTER));
		CHAR sNameBuf[60];
		memcpy(sNameBuf, lpConnection->sName, strlen(lpConnection->sName) + 1);
		strcat(cp.output, horizontal_align_text(sNameBuf, sNameLength, ' ', TA_HORIZONTAL_CENTER));
		strcat(cp.output, horizontal_align_text(inet_ntoa(lpConnection->addr.sin_addr), ipLength, ' ', TA_HORIZONTAL_CENTER));
		strcat(cp.output, horizontal_align_text(itoa(ntohs(lpConnection->addr.sin_port), buffer, 10), portLength, ' ', TA_HORIZONTAL_CENTER));

		memset(buffer, 0, sizeof(buffer));
		strftime(buffer, sizeof(buffer), "%d.%m.%y/%H:%M:%S", localtime(&lpConnection->tStart));
		printf("%s", buffer);
		strcat(cp.output, horizontal_align_text(buffer, timeLength, ' ', TA_HORIZONTAL_CENTER));
		strcat(cp.output, "\n");
	}

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

BOOL CommandLibset(CONSOLE_PIPE cp, LPCSTR argv) {
	DISPATCH_SERVER ds = cp.ds;

	LPSTR command = argv;
	//LPSTR libName = trimWord(command);

	if (!strcmp(command, "ps")) {

	}
	else if (!strcmp(command, "lock")) {

	}
	else if (!strcmp(command, "unlock")) {

	}
	else if (!strcmp(command, "fixed")) {

	}
	else {
		return FALSE;
	}
	return TRUE;
}