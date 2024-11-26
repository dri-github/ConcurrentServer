﻿// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
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
#include "../ConcurrentServer/connection.c"

extern "C"
{
    __declspec(dllexport) void Service(LPVOID lpParam)
    {
        LPCONNECTION lpConnection = (LPCONNECTION)lpParam;

        printf("[EchoService] Status: Start\n");
        CHAR text[64];
        memset(text, 0, sizeof(text));
        while (recv(lpConnection->s, text, sizeof(text), NULL) != SOCKET_ERROR) {
            printf("[EchoService] Status: Reply \"%s\"\n", text);
            if (strcmp(text, "") == 0)
                break;
            UpdateTimer(lpConnection);
            if (send(lpConnection->s, text, strlen(text) + 1, NULL) == SOCKET_ERROR)
                break;
        }
        
        lpConnection->state = CONNECTION_STATE_DROPED;
        HANDLE hEvent;
        if ((hEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, DISPATCH_SERVER_EVENT_NAME)) != NULL) {
            SetEvent(hEvent);
        }
    }
}