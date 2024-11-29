#include "./lib_loader.h"

HANDLE LibLoaderLoad(LPLIB_LOADER lpLdr, LPCSTR libName) {
	HANDLE hLib;
	if ((hLib = LoadLibraryA(libName)) == NULL) {
		return NULL;
	}

	LPLOADED_LIB lpLib;
	if ((lpLib = (LPLOADED_LIB)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOADED_LIB))) == NULL) {
		return NULL;
	}

	lpLib->handle = hLib;
	lpLib->name = libName;
	lpLib->refCount = 0;
	lpLib->state = LIB_STATE_LOADED;

	ForwardListPushFront(lpLdr->lpLibs, lpLib);
	return hLib;
}

HANDLE LibLoaderGet(LPLIB_LOADER lpLdr, LPCSTR libName) {
	LPLOADED_LIB lpLib;
	if ((lpLib = LibLoaderFind(lpLdr, libName)) != NULL) {
		if (lpLib->state == LIB_STATE_LOCKED || lpLib->state == LIB_STATE_WAITLOCKED)
			return NULL;
		return lpLib->handle;
	}

	return LibLoaderLoad(lpLdr, libName);
}

LPLOADED_LIB LibLoaderFind(LPLIB_LOADER lpLdr, LPCSTR libName) {
	LPFORWARD_LIST_NODE it = lpLdr->lpLibs;
	while ((it = it->Next) != NULL) {
		LPLOADED_LIB lpLib = it->Data;
		if (!strcmp(lpLib->name, libName)) {
			return lpLib;
		}
	}

	return NULL;
}

BOOL LibLoaderSetState(LPLIB_LOADER lpLdr, LPCSTR libName, DWORD state) {
	LPLOADED_LIB lpLib;
	if ((lpLib = LibLoaderFind(lpLdr, libName)) != NULL) {
		
	}
	else {
		HANDLE hLib = NULL;
		if (state == LIB_STATE_LOADED || state == LIB_STATE_FIXED) {
			if ((hLib = LoadLibraryA(libName)) == NULL) {
				return FALSE;
			}
		}

		if ((lpLib = (LPLOADED_LIB)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOADED_LIB))) == NULL) {
			return FALSE;
		}

		lpLib->handle = hLib;
		lpLib->name = libName;
		lpLib->refCount = 0;

		ForwardListPushFront(lpLdr->lpLibs, lpLib);
	}

	switch (state) {
	case LIB_STATE_LOADED:
	case LIB_STATE_FIXED:
		if (lpLib->handle == NULL) {
			HANDLE hLib;
			if ((hLib = LoadLibraryA(libName)) == NULL) {
				return FALSE;
			}
			lpLib->handle = hLib;
		}
		break;
	case LIB_STATE_LOCKED:
		if (lpLib->handle != NULL) {
			FreeLibrary(lpLib->handle);
			lpLib->handle = NULL;
		}
		break;
	case LIB_STATE_WAITLOCKED:
		break;
	}

	lpLib->state = state;

	return TRUE;
}

BOOL LibloaderUnload(LPLIB_LOADER lpLdr, LPCSTR libName) {
	LPFORWARD_LIST_NODE it = lpLdr->lpLibs;
	LPFORWARD_LIST_NODE itPrev = it;
	while ((it = it->Next) != NULL) {
		LPLOADED_LIB lpLib = it->Data;
		if (!strcmp(lpLib->name, libName)) {
			if (lpLib->refCount > 0)
				lpLib->refCount--;

			switch (lpLib->state) {
			case LIB_STATE_LOADED:
				if (lpLib->refCount == 0) {
					itPrev->Next = it->Next;

					FreeLibrary(lpLib->handle);
					HeapFree(GetProcessHeap(), 0, lpLib->name);
					HeapFree(GetProcessHeap(), 0, lpLib);
					HeapFree(GetProcessHeap(), 0, it);

					return TRUE;
				}
				break;
			case LIB_STATE_WAITLOCKED:
				if (lpLib->refCount == 0) {
					FreeLibrary(lpLib->handle);
					lpLib->handle = NULL;

					return TRUE;
				}
				break;
			case LIB_STATE_LOCKED:
			case LIB_STATE_FIXED:
				return FALSE;
			default:
				return FALSE;
			}

			break;
		}

		itPrev = it;
	}

	return FALSE;
}