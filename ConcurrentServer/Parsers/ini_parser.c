#include "./ini_parser.h"

int ini_parse(const char* filename, LPFORWARD_LIST_NODE sectionsList) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    char current_section[MAX_LINE_LENGTH] = "";

    while (fgets(line, sizeof(line), file)) {
        char* trimmed_line = trim(line);

        if (trimmed_line[0] == ';' || trimmed_line[0] == '#' || trimmed_line[0] == '\0') {
            continue;
        }

        if (trimmed_line[0] == '[') {
            char* end = strchr(trimmed_line, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, trimmed_line + 1, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0';

                LPSECTION_CONTAINER lpSection;
                if ((lpSection = (LPSECTION_CONTAINER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SECTION_CONTAINER))) == NULL) {
                    return -2;
                }
                if ((lpSection->name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(current_section) + 1) * sizeof(CHAR))) == NULL) {
                    return -2;
                }
                memcpy(lpSection->name, current_section, strlen(current_section) + 1);
                lpSection->keyValues = ForwardListCreateNode(NULL);
                ForwardListPushFront(sectionsList, lpSection);
            }
            else {
                return -3;
            }
        }
        else {
            char* delimiter = strchr(trimmed_line, '=');
            if (delimiter) {
                *delimiter = '\0';
                char* key = trim(trimmed_line);
                char* value = trim(delimiter + 1);
                LPSECTION_CONTAINER keyValues = (LPSECTION_CONTAINER)sectionsList->Next->Data;
                LPKEY_VALUE keyValue;
                if ((keyValue = (LPKEY_VALUE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(KEY_VALUE))) == NULL) {
                    return -2;
                }

                if ((keyValue->key = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(key) + 1) * sizeof(CHAR))) == NULL) {
                    return -2;
                }
                memcpy(keyValue->key, key, strlen(key) + 1);
                if ((keyValue->value = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(value) + 1) * sizeof(CHAR))) == NULL) {
                    return -2;
                }
                memcpy(keyValue->value, value, strlen(value) + 1);
                ForwardListPushFront(keyValues->keyValues, keyValue);
            }
            else {
                return -3;
            }
        }
    }

    fclose(file);

    return 0;
}

LPSECTION_CONTAINER find_section(LPFORWARD_LIST_NODE lpList, LPCSTR name) {
    LPFORWARD_LIST_NODE it = lpList;
    while ((it = it->Next) != NULL) {
        LPSECTION_CONTAINER section = it->Data;
        if (!strcmp(section->name, name)) {
            return section;
        }
    }
}

VOID word_trim_parse(LPSTR text, CHAR separator, VOID(*wordCheck)(LPCSTR word, LPVOID lpParam), LPVOID lpParam) {
    LPSTR value = text;
    for (LPSTR currentAServer = value; *currentAServer != '\0'; currentAServer++) {
        if (*currentAServer == separator) {
            *currentAServer = '\0';

            while (*value == ' ') value++;
            wordCheck(value, lpParam);
            value = currentAServer + 1;
        }
    }

    while (*value == ' ') value++;
    wordCheck(value, lpParam);
}