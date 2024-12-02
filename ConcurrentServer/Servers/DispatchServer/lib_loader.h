#ifndef LIB_LOADER_H
#define LIB_LOADER_H

#include "../../Base/forward_list.h"

#define LIB_STATE_LOADED 0
#define LIB_STATE_LOCKED 1
#define LIB_STATE_WAITLOCKED 2
#define LIB_STATE_FIXED 3

typedef struct _LOADED_LIB {
	HANDLE handle;
	LPCSTR name;
	INT refCount;
	DWORD state;
} LOADED_LIB, * LPLOADED_LIB;

typedef struct _LIB_LOADER {
	LPFORWARD_LIST_NODE lpLibs;
} LIB_LOADER, *LPLIB_LOADER;

HANDLE LibLoaderLoad(LPLIB_LOADER lpLdr, LPCSTR libName);
HANDLE LibLoaderGet(LPLIB_LOADER lpLdr, LPCSTR libName);
LPLOADED_LIB LibLoaderFind(LPLIB_LOADER lpLdr, LPCSTR libName);
BOOL LibLoaderSetState(LPLIB_LOADER lpLdr, LPCSTR libName, DWORD state);
BOOL LibloaderUnload(LPLIB_LOADER lpLdr, LPCSTR libName);

#endif // !LIB_LOADER_H