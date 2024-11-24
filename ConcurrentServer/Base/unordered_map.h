#ifndef UNORDERED_MAP_H
#define UNORDERED_MAP_H

#include "./forward_list.h"

typedef struct _UNORDERED_MAP_NODE {
	DWORD hash;
	SIZE_T count;
	LPVOID data;
} UNORDERED_MAP_NODE, *LPUNORDERED_MAP_NODE;

SIZE_T UnorderedMapCount(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID));
LPVOID UnorderedMapCountedGet(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID));
BOOL UnorderedMapCountedRemove(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID));

#endif // !UNORDERED_MAP_H