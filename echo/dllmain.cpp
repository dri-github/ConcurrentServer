// dllmain.cpp : Определяет точку входа для приложения DLL.
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

#include <stdio.h>

#include "../ConcurrentServer/accept_server.h"

extern "C"
{
    __declspec(dllexport) void Service(LPVOID lpParam)
    {
        LPCONNECTION lpConnection = (LPCONNECTION)lpParam;
        LPSTR text = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHAR) * 30);
        strcpy(text, "Hello From Client 10");
        do {
            Sleep(100);
            send(lpConnection->s, text, strlen(text) + 1, NULL);
        } while (recv(lpConnection->s, text, strlen(text) + 1, NULL) != SOCKET_ERROR);
        HeapFree(GetProcessHeap(), 0, text);
        
        closesocket(lpConnection->s);
    }
}