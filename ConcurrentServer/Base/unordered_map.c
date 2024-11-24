#include "./unordered_map.h"

SIZE_T UnorderedMapCount(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID)) {
	LPFORWARD_LIST_NODE it = lpMap;
	while ((it = it->Next) != NULL) {
		LPUNORDERED_MAP_NODE mapNode = (LPUNORDERED_MAP_NODE)it;
		if (hashFunc(key) == mapNode->hash)
			return mapNode->count;
	}

	return 0;
}

LPVOID UnorderedMapCountedGet(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID)) {
	LPFORWARD_LIST_NODE it = lpMap;
	while ((it = it->Next) != NULL) {
		LPUNORDERED_MAP_NODE mapNode = (LPUNORDERED_MAP_NODE)it;
		if (hashFunc(key) == mapNode->hash) {
			mapNode->count++;
			return mapNode->data;
		}
	}

	LPUNORDERED_MAP_NODE newNode;
	if ((newNode = (LPUNORDERED_MAP_NODE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(UNORDERED_MAP_NODE))) == NULL) {
		return NULL;
	}
	newNode->hash = hashFunc(key);
	newNode->data = NULL;
	newNode->count = 1;
	ForwardListPushFront(lpMap, newNode);

	return newNode;
}

BOOL UnorderedMapCountedRemove(LPFORWARD_LIST_NODE lpMap, LPVOID key, DWORD(*hashFunc)(LPVOID)) {
	LPFORWARD_LIST_NODE it = lpMap;
	LPFORWARD_LIST_NODE itPrev = it;
	while ((it = it->Next) != NULL) {
		LPUNORDERED_MAP_NODE mapNode = (LPUNORDERED_MAP_NODE)it;
		if (hashFunc(key) == mapNode->hash) {
			if ((--mapNode) == 0) {
				HeapFree(GetProcessHeap(), 0, mapNode->data);
				itPrev->Next = it->Next;
				HeapFree(GetProcessHeap(), 0, it);
			}

			return TRUE;
		}

		itPrev = it;
	}

	return FALSE;
}