#ifndef INI_PARSER_H
#define INI_PARSER_H

#include "../UI/ui.h"
#include "../Base/forward_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

typedef struct _KEY_VALUE {
    LPSTR key;
    LPSTR value;
} KEY_VALUE, *LPKEY_VALUE;

typedef struct _SECTION_CONTAINER {
    LPSTR name;
    LPFORWARD_LIST_NODE keyValues;
} SECTION_CONTAINER, *LPSECTION_CONTAINER;

int ini_parse(const char* filename, LPFORWARD_LIST_NODE sectionsList);
LPSECTION_CONTAINER find_section(LPFORWARD_LIST_NODE lpList, LPCSTR name);

VOID word_trim_parse(LPSTR text, CHAR separator, VOID(*wordCheck)(LPCSTR word, LPVOID lpParam), LPVOID lpParam);

#endif // !INI_PARSER_H