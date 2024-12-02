#include "./acserv.h"

#include "../../../../UI/ui.h"

BOOL CommandAcServ(CONSOLE_PIPE cp, LPCSTR argv) {
	LPFORWARD_LIST_NODE ass = cp.ass;

	LPSTR command = trimWord(argv);

	if (!strcmp(argv, "ps")) {
		char_column columns[3] = {
			{ 24, ' ', TA_HORIZONTAL_LEFT },
			{ 10, ' ', TA_HORIZONTAL_CENTER },
			{ 18, ' ', TA_HORIZONTAL_CENTER }
		};

		LPCSTR** array = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 20 * sizeof(LPCSTR*));
		for (int i = 0; i < 20; i++) {
			array[i] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 3 * sizeof(LPCSTR));
			for (int j = 0; j < 3; j++) {
				array[i][j] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 30 * sizeof(CHAR));
			}
		}

#define SetColumn(id, name) memcpy(array[0][id], name, strlen(name) + 1)
		SetColumn(0, "IP");
		SetColumn(1, "Port");
		SetColumn(2, "State");

		char_table table;
		table.columns_count = 3;
		table.columns = columns;
		table.array = array;

		INT i = 1;
		LPFORWARD_LIST_NODE it = ass;
		while ((it = it->Next) != NULL && i < 20) {
			LPACCEPT_SERVER lpAcServ = it->Data;
			SOCKADDR_IN addr = lpAcServ->addr;

			array[i][0] = inet_ntoa(addr.sin_addr);

			CHAR buffer[64];
			memset(buffer, 0, sizeof(buffer));
			itoa(ntohs(addr.sin_port), buffer, 10);
			memcpy(array[i][1], buffer, strlen(buffer) + 1);

			switch (lpAcServ->state) {
			case ASERV_STATE_CLOSE:
				array[i][2] = "CLOSE";
				break;
			case ASERV_STATE_START:
				array[i][2] = "START";
				break;
			case ASERV_STATE_STOP:
				array[i][2] = "STOP";
				break;
			default:
				break;
			}

			i++;
		}
		table.lines_count = i;

		draw_table(table, cp.output);

		for (int i = 0; i < 20; i++) {
			HeapFree(GetProcessHeap(), 0, array[i]);
		}
		HeapFree(GetProcessHeap(), 0, array);
	}
	else if (!strcmp(argv, "start")) {
		SetAllAcServ(ass, ASERV_STATE_START);
	}
	else if (!strcmp(argv, "stop")) {
		SetAllAcServ(ass, ASERV_STATE_STOP);
	}
	else if (!strcmp(argv, "state")) {
		DWORD state = atoi(trimWord(command));
		INT port = atoi(command);

		LPFORWARD_LIST_NODE it = ass;
		LPFORWARD_LIST_NODE itPrev = it;
		while ((it = it->Next) != NULL) {
			LPACCEPT_SERVER lpAs = it->Data;

			if (ntohs(lpAs->addr.sin_port) == port) {
				switch (state) {
				case ASERV_STATE_CLOSE:
					CloseAcceptServer(lpAs);
					closesocket(lpAs->s);

					TerminateThread(lpAs->hThread, 0);
					itPrev->Next = it->Next;
					CloseHandle(lpAs->hThread);
					HeapFree(GetProcessHeap(), 0, it->Data);
					HeapFree(GetProcessHeap(), 0, it);
					it = itPrev;

					return TRUE;
					break;
				case ASERV_STATE_START:
					return StartAcceptServer(lpAs);
					break;
				case ASERV_STATE_STOP:
					return StopAcceptServer(lpAs);
					break;
				default:
					return FALSE;
				}
				break;
			}

			itPrev = it;
		}

		LPACCEPT_SERVER lpServer;
		if ((lpServer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACCEPT_SERVER))) == NULL) {
			return FALSE;
		}

		SOCKADDR_IN addr;

		// Default values
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		lpServer->addr = addr;
		lpServer->connections = cp.ds.connections;
		lpServer->cs = cp.ds.cs;

		if (!OpenAcceptServer(lpServer)) {
			HeapFree(GetProcessHeap(), 0, lpServer);
			return FALSE;
		}
		else {
			lpServer->state = state;
			ForwardListPushFront(ass, lpServer);
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

BOOL SetAllAcServ(LPFORWARD_LIST_NODE lpAss, DWORD state) {
	BOOL result = TRUE;

	LPFORWARD_LIST_NODE it = lpAss;
	while ((it = it->Next) != NULL) {
		LPACCEPT_SERVER lpAs = it->Data;

		switch (state) {
		case ASERV_STATE_CLOSE:
			if (!CloseAcceptServer(lpAs)) result = FALSE;
			break;
		case ASERV_STATE_START:
			if (!StartAcceptServer(lpAs)) result = FALSE;
			break;
		case ASERV_STATE_STOP:
			if (!StopAcceptServer(lpAs)) result = FALSE;
			break;
		default:
			return FALSE;
		}
	}

	return result;
}