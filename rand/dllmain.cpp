// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"

#include <random>
#include <ctime>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    srand(time(NULL));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable: 4996)

#define DISPATCH_SERVER_EVENT_NAME "DispatchServer"

#include "../ConcurrentServer/connection.h"

extern "C"
{
    __declspec(dllexport) void Service(LPVOID lpParam)
    {
        LPCONNECTION lpConnection = (LPCONNECTION)lpParam;

        printf("[RandService] Status: Start\n");
        CHAR text[100];
        memset(text, 0, sizeof(text));
        while (recv(lpConnection->s, text, sizeof(text), NULL) != SOCKET_ERROR) {
            if (strcmp(text, "rand")) {
                break;
            }

            memset(text, 0, sizeof(text));
            CHAR buffer[100];
            LPSTR res = itoa(rand(), buffer, 10);
            memcpy(text, res, strlen(res) + 1);

            InterlockedExchange((unsigned long long*) & lpConnection->tChange, time(NULL));
            if (send(lpConnection->s, text, strlen(text) + 1, NULL) == SOCKET_ERROR)
                break;
        }

        InterlockedExchange(&lpConnection->state, CONNECTION_STATE_DROPED);
        HANDLE hEvent;
        if ((hEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) != NULL) {
            SetEvent(hEvent);
        }
    }
}