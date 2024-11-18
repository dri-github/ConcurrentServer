#ifndef FORWARD_LIST_H
#define FORWARD_LIST_H

#include <windows.h>

typedef struct _FORWARD_LIST_NODE {
    LPVOID Data;
    struct _FORWARD_LIST_NODE* Next;
} FORWARD_LIST_NODE, * LPFORWARD_LIST_NODE;

LPFORWARD_LIST_NODE ForwardListCreateNode(LPVOID data);
BOOL ForwardListPushFront(LPFORWARD_LIST_NODE* lppHead, LPVOID data);
BOOL ForwardListPushBack(LPFORWARD_LIST_NODE* lppHead, LPVOID data);
BOOL ForwardListDeleteNode(LPFORWARD_LIST_NODE* lppHead, LPVOID key, BOOL(*compare)(LPVOID, LPVOID));
VOID ForwardListFree(LPFORWARD_LIST_NODE lpHead, VOID(*freeData)(LPVOID));

#endif // FORWARD_LIST_H