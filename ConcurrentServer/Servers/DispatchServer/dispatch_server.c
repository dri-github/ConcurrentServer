#include "./dispatch_server.h"

#include "../accept_server.h"

#include <stdio.h>
#include <time.h>

DWORD WINAPI DispatchServer(LPVOID lpParam) {
	LPDISPATCH_SERVER server = lpParam;
	LPFORWARD_LIST_NODE connections = server->connections;
	LPLIB_LOADER lpLibLoader = server->lpLibLoader;

	HANDLE hAddConnection;
	if ((hAddConnection = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) == NULL) {
		return -1;
	}

	LPFORWARD_LIST_NODE libList = ForwardListCreateNode(NULL);

	HANDLE hTimer;
	if ((hTimer = CreateWaitableTimerA(NULL, TRUE, TIMER_NAME)) == NULL) {
		return -1;
	}
	LARGE_INTEGER liDueTime;

	while (TRUE) {
		DWORD dwWaitResult;
		while ((dwWaitResult = WaitForSingleObject(hAddConnection, 1)) != WAIT_OBJECT_0 &&
			   (dwWaitResult = WaitForSingleObject(hTimer, 1)) != WAIT_OBJECT_0);

		TIME_T tCurrent = time(NULL);
		printf("[DispatchServer] Status: Updating\n");

		TIME_T nearlyTime = 5;
		if (connections->Next == NULL) {
			liDueTime.QuadPart = -TIMER_SECOND * nearlyTime;
			if (!SetWaitableTimer(hTimer, &liDueTime, NULL, NULL, NULL, FALSE)) {
				printf("[DispatchServer] Error: SetWaitableTimer by connections count 0\n");
			}
			else {
				printf("[DispatchServer] Status: connections count 0. SetWaitableTimer = %i s\n", (int)(nearlyTime / TIMER_SECOND));
			}
		}

		EnterCriticalSection(server->cs);
		LPFORWARD_LIST_NODE it = connections;
		LPFORWARD_LIST_NODE itPrev = it;
		while ((it = it->Next) != NULL) {
			LPCONNECTION connection = it->Data;
			switch (connection->state) {
			case CONNECTION_STATE_ACCEPTED: {
					connection->state = CONNECTION_STATE_SUCCESS;
					connection->tChange = connection->tStart = time(NULL);
					connection->tWait = 60;
					printf("[DispatchServer] Status: Changing service for %i\n", ntohs(connection->addr.sin_port));

					LPSTR buffer[MAX_SIZE_SERVICE_NAME + 1];
					memset(buffer, 0, sizeof(buffer));
					INT lenght = 0;

					if ((lenght = recv(connection->s, buffer, sizeof(buffer), NULL)) == SOCKET_ERROR) {
						printf("[DispatchServer] Error: recv");
						break;
					}

					LPSTR sName;
					SIZE_T sNameLength = strlen(buffer) + strlen(SERVICE_PATH_PREFIX) + strlen(SERVICE_NAME_POSTFIX);
					if ((sName = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sNameLength + 1)) == NULL) {
						continue;
					}
					
					strcpy(sName, SERVICE_PATH_PREFIX);
					strcat(sName, buffer);
					strcat(sName, SERVICE_NAME_POSTFIX);
					sName[sNameLength] = '\0';

					HANDLE hService = LibLoaderGet(lpLibLoader, sName);
					if (hService == NULL) {
						CHAR error[] = "[DispatchServer] Warning: Service name not exist\n";
						printf("[DispatchServer] Warning: Service name \"%s\" not exist\n", sName);
						printf("[DispatchServer] Status: Drop connection for %i\n", ntohs(connection->addr.sin_port));
						send(connection->s, error, strlen(error) + 1, NULL);
						closesocket(connection->s);
						connection->state = CONNECTION_STATE_SUCCESS;
						continue;
					}

					LPSERVICE_FUNCTION lpFunction = (LPSERVICE_FUNCTION)GetProcAddress(hService, SERVICE_FUNCTION_NAME);
					if (lpFunction != NULL) {
						connection->sName = sName;
						send(connection->s, buffer, sizeof(buffer), NULL);
						printf("[DispatchServer] Status: The connection %i select service \"%s\"\n", ntohs(connection->addr.sin_port), sName);
						CreateThread(NULL, NULL, lpFunction, connection, NULL, NULL);
					}
					break;
				}
			case CONNECTION_STATE_SUCCESS: {
				TIME_T dt = connection->tWait - (tCurrent - connection->tChange);
				if (dt < nearlyTime)
					nearlyTime = dt;

				if (tCurrent - connection->tChange >= connection->tWait) {
					printf("[DispatchServer] Status: Droped connection %i by timer\n", ntohs(connection->addr.sin_port));

					closesocket(connection->s);
					itPrev->Next = it->Next;
					LibloaderUnload(lpLibLoader, connection->sName);
					HeapFree(GetProcessHeap(), 0, it->Data);
					HeapFree(GetProcessHeap(), 0, it);
					it = itPrev;
				}
				break;
			}
			case CONNECTION_STATE_DROPED:
				printf("[DispatchServer] Status: Droped connection %i\n", ntohs(connection->addr.sin_port));

				closesocket(connection->s);
				itPrev->Next = it->Next;
				LibloaderUnload(lpLibLoader, connection->sName);
				HeapFree(GetProcessHeap(), 0, it->Data);
				HeapFree(GetProcessHeap(), 0, it);
				it = itPrev;
				break;
			default:
				break;
			}

			itPrev = it;
		}

		nearlyTime++;
		liDueTime.QuadPart = -TIMER_SECOND * nearlyTime;
		if (!SetWaitableTimer(hTimer, &liDueTime, NULL, NULL, NULL, FALSE)) {
			printf("[DispatchServer] Error: SetWaitableTimer\n");
		}
		LeaveCriticalSection(server->cs);

		ResetEvent(hAddConnection);
	}

	return 0;
}