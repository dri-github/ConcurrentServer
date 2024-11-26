#include "./connection.h"

LARGE_INTEGER liDueTime;

BOOL WINAPI UpdateTimer(LPCONNECTION cn) {
	HANDLE hTimer;
	if ((hTimer = OpenWaitableTimerA(TIMER_ALL_ACCESS, FALSE, TIMER_NAME)) == NULL) {
		return FALSE;
	}
	
	liDueTime.QuadPart = -TIMER_SECOND * cn->tWait;
	if (!SetWaitableTimer(hTimer, &liDueTime, NULL, NULL, NULL, FALSE)) {
		return FALSE;
	}

	cn->tChange = time(NULL);
	return TRUE;
}