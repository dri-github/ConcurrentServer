#include "./libctl.h"

#include "../../../../UI/ui.h"

BOOL CommandLibCtl(CONSOLE_PIPE cp, LPCSTR argv) {
	DISPATCH_SERVER ds = cp.ds;

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
		SetColumn(0, "Name");
		SetColumn(1, "RefCount");
		SetColumn(2, "State");

		char_table table;
		table.columns_count = 3;
		table.columns = columns;
		table.array = array;

		INT i = 1;
		LPFORWARD_LIST_NODE it = ds.lpLibLoader->lpLibs;
		while ((it = it->Next) != NULL && i < 20) {
			LPLOADED_LIB lpLib = it->Data;

			array[i][0] = lpLib->name;

			CHAR buffer[64];
			memset(buffer, 0, sizeof(buffer));
			itoa(lpLib->refCount, buffer, 10);
			memcpy(array[i][1], buffer, strlen(buffer) + 1);

			switch (lpLib->state) {
			case LIB_STATE_LOADED:
				array[i][2] = "LOADED";
				break;
			case LIB_STATE_FIXED:
				array[i][2] = "FIXED";
				break;
			case LIB_STATE_LOCKED:
				array[i][2] = "LOCKED";
				break;
			case LIB_STATE_WAITLOCKED:
				array[i][2] = "WAITLOCKED";
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
	else if (!strcmp(argv, "state")) {
		LPSTR libName = trimWord(command);

		LPSTR commandCopy = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(command) + 1) * sizeof(CHAR));
		memcpy(commandCopy, command, strlen(command) + 1);

		LPFORWARD_LIST_NODE it = ds.lpLibLoader->lpLibs;
		while ((it = it->Next) != NULL) {
			LPLOADED_LIB lpLib = it->Data;

			if (!strcmp(lpLib->name, command)) {
				if (atoi(libName) == LIB_STATE_LOCKED) {
					CloseConnectionsOnService(ds.connections, ds, command);
				}
				
				if (!LibLoaderSetState(ds.lpLibLoader, commandCopy, atoi(libName))) {
					strcat(cp.output, "Error of set status\n");
					return FALSE;
				}
				else {
					strcat(cp.output, "Status was set\n");
					return TRUE;
				}
				break;
			}
		}

		if (!LibLoaderSetState(ds.lpLibLoader, commandCopy, atoi(libName))) {
			strcat(cp.output, "Lib with name ");
			strcat(cp.output, command);
			strcat(cp.output, " not exist\n");
			return FALSE;
		}
		else {
			strcat(cp.output, "Status was set\n");
			return TRUE;
		}
	}
	else {
		return FALSE;
	}
	return TRUE;
}

VOID CloseConnectionsOnService(LPFORWARD_LIST_NODE connections, DISPATCH_SERVER ds, LPCSTR serviceName) {
	LPFORWARD_LIST_NODE itConn = connections;
	LPFORWARD_LIST_NODE itPrevConn = itConn;
	EnterCriticalSection(ds.cs);
	while ((itConn = itConn->Next) != NULL) {
		LPCONNECTION lpConnection = itConn->Data;
		printf("Connection droped %s %i\n", lpConnection->sName, strcmp(lpConnection->sName, serviceName));
		if (!strcmp(lpConnection->sName, serviceName)) {
			closesocket(lpConnection->s);
			itPrevConn->Next = itConn->Next;
			LibloaderUnload(ds.lpLibLoader, serviceName);
			HeapFree(GetProcessHeap(), 0, itConn->Data);
			HeapFree(GetProcessHeap(), 0, itConn);
			itConn = itPrevConn;
		}

		itPrevConn = itConn;
	}
	LeaveCriticalSection(ds.cs);
}