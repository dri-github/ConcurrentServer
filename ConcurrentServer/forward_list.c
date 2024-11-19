#include "forward_list.h"

LPFORWARD_LIST_NODE ForwardListCreateNode(LPVOID data) {
    LPFORWARD_LIST_NODE lpNode = (LPFORWARD_LIST_NODE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FORWARD_LIST_NODE));
    if (lpNode == NULL) {
        return NULL;
    }
    
    lpNode->Data = data;
    lpNode->Next = NULL;
    return lpNode;
}

BOOL ForwardListPushFront(LPFORWARD_LIST_NODE lpHead, LPVOID data) {
    LPFORWARD_LIST_NODE lpNode = ForwardListCreateNode(data);
    if (lpHead == NULL || lpNode == NULL) {
        return FALSE;
    }

    lpNode->Next = lpHead->Next;
    lpHead->Next = lpNode;
    return TRUE;
}

BOOL ForwardListPushBack(LPFORWARD_LIST_NODE lpHead, LPVOID data) {
    LPFORWARD_LIST_NODE lpNode = ForwardListCreateNode(data);
    if (lpHead == NULL || lpNode == NULL) {
        return FALSE;
    }

    LPFORWARD_LIST_NODE lpCurrent = lpHead;
    while (lpCurrent->Next != NULL) {
        lpCurrent = lpCurrent->Next;
    }
    lpCurrent->Next = lpNode;
    return TRUE;
}

BOOL ForwardListDeleteNode(LPFORWARD_LIST_NODE lpHead, LPVOID key, BOOL(*compare)(LPVOID, LPVOID), VOID(*freeData)(LPVOID)) {
    if (lpHead == NULL) {
        return FALSE;
    }

    LPFORWARD_LIST_NODE lpCurrent = lpHead->Next;
    LPFORWARD_LIST_NODE lpPrev = NULL;
    while (lpCurrent != NULL && !compare(lpCurrent->Data, key)) {
        lpPrev = lpCurrent;
        lpCurrent = lpCurrent->Next;
    }

    if (lpPrev == NULL || lpCurrent == NULL) {
        return FALSE;
    }

    lpPrev->Next = lpCurrent->Next;
    if (freeData != NULL) {
        freeData(lpCurrent->Data);
    }
    HeapFree(GetProcessHeap(), 0, lpCurrent);
    return TRUE;
}

VOID ForwardListFree(LPFORWARD_LIST_NODE lpHead, VOID(*freeData)(LPVOID)) {
    LPFORWARD_LIST_NODE lpCurrent = lpHead;
    while (lpCurrent != NULL) {
        LPFORWARD_LIST_NODE lpTemp = lpCurrent;
        lpCurrent = lpCurrent->Next;
        if (freeData != NULL) {
            freeData(lpTemp->Data);
        }
        HeapFree(GetProcessHeap(), 0, lpTemp);
    }
}