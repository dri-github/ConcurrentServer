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

BOOL ForwardListPushFront(LPFORWARD_LIST_NODE* lppHead, LPVOID data) {
    if (lppHead == NULL) {
        return FALSE;
    }
    LPFORWARD_LIST_NODE lpNode = ForwardListCreateNode(data);
    if (lpNode == NULL) {
        return FALSE;
    }
    lpNode->Next = *lppHead;
    *lppHead = lpNode;
    return TRUE;
}

BOOL ForwardListPushBack(LPFORWARD_LIST_NODE* lppHead, LPVOID data) {
    if (lppHead == NULL) {
        return FALSE;
    }
    LPFORWARD_LIST_NODE lpNode = ForwardListCreateNode(data);
    if (lpNode == NULL) {
        return FALSE;
    }
    if (*lppHead == NULL) {
        *lppHead = lpNode;
        return TRUE;
    }
    LPFORWARD_LIST_NODE lpCurrent = *lppHead;
    while (lpCurrent->Next != NULL) {
        lpCurrent = lpCurrent->Next;
    }
    lpCurrent->Next = lpNode;
    return TRUE;
}

BOOL ForwardListDeleteNode(LPFORWARD_LIST_NODE* lppHead, LPVOID key, BOOL(*compare)(LPVOID, LPVOID)) {
    if (lppHead == NULL || *lppHead == NULL) {
        return FALSE;
    }
    LPFORWARD_LIST_NODE lpCurrent = *lppHead;
    LPFORWARD_LIST_NODE lpPrev = NULL;

    if (compare(lpCurrent->Data, key)) {
        *lppHead = lpCurrent->Next;
        HeapFree(GetProcessHeap(), 0, lpCurrent);
        return TRUE;
    }

    while (lpCurrent != NULL && !compare(lpCurrent->Data, key)) {
        lpPrev = lpCurrent;
        lpCurrent = lpCurrent->Next;
    }

    if (lpPrev == NULL || lpCurrent == NULL) {
        return FALSE;
    }

    lpPrev->Next = lpCurrent->Next;
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