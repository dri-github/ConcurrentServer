#ifndef CONNECTION_H
#define CONNECTION_H

#include <Windows.h>
#include <time.h>

#define CONNECTION_STATE_ACCEPTED 0
#define CONNECTION_STATE_SUCCESS 1
#define CONNECTION_STATE_DROPED 2
#define CONNECTION_STATE_ERROR 3

#define TIMER_SECOND 10000000LL
#define TIMER_NAME "Dispatch"

typedef time_t TIME_T;

typedef struct _CONNECTION {
	SOCKET s;
	SOCKADDR_IN addr;
	DWORD state;
	LPCSTR sName;
	TIME_T tWait;
	TIME_T tChange;
	TIME_T tStart;
} CONNECTION, * LPCONNECTION;

BOOL WINAPI UpdateTimer(LPCONNECTION cn);

#endif // !CONNECTION_H