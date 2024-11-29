#ifndef DISPATCH_SERVER_H
#define DISPATCH_SERVER_H

#include "../../Base/forward_list.h"
#include "./lib_loader.h"

#include <Windows.h>
#pragma warning(disable: 4996)

#define DISPATCH_SERVER_EVENT_NAME "DispatchServer"

#define MAX_SIZE_SERVICE_NAME 16
#define SERVICE_PATH_PREFIX "./Services/"
#define SERVICE_NAME_POSTFIX ".dll"
#define SERVICE_FUNCTION_NAME "Service"
#define CONSOLE_PIPE_OUT_SIZE 2048

typedef DWORD (WINAPI *LPSERVICE_FUNCTION)(LPVOID lpParam);

//#define LIB_STATE_LOADED 0b0000
//#define LIB_STATE_LOCKED 0b0000
//#define LIB_STATE_WAITLOCKED 0b0000
//#define LIB_STATE_FIXED 0b0000
//
//typedef struct _LOADED_LIB {
//	HANDLE handle;
//	LPCSTR name;
//	INT refCount;
//	DWORD state;
//} LOADED_LIB, *LPLOADED_LIB;

typedef struct _DISPATCH_SERVER {
	LPFORWARD_LIST_NODE connections;
    LPLIB_LOADER lpLibLoader;
	LPCRITICAL_SECTION cs;
	LPCSTR postfix;
	HANDLE hThread;
} DISPATCH_SERVER, *LPDISPATCH_SERVER;

DWORD WINAPI DispatchServer(LPVOID lpParam);
HANDLE AddServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
HANDLE FindServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
HANDLE FindAndLoadServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);
VOID DeleteServiceLib(LPFORWARD_LIST_NODE lpLibList, LPCSTR name);

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

// Удаляет пробелы в начале и конце строки
char *trim(char *str) {
    char *end;

    // Удаляем пробелы в начале
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // Если строка пуста
        return str;

    // Удаляем пробелы в конце
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Завершаем строку
    *(end + 1) = '\0';

    return str;
}

// Парсинг INI-файла
void parse_ini(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    char line[MAX_LINE_LENGTH];
    char current_section[MAX_LINE_LENGTH] = "";

    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = trim(line);

        // Игнорируем комментарии и пустые строки
        if (trimmed_line[0] == ';' || trimmed_line[0] == '#' || trimmed_line[0] == '\0') {
            continue;
        }

        // Если это секция
        if (trimmed_line[0] == '[') {
            char *end = strchr(trimmed_line, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, trimmed_line + 1, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0'; // Для безопасности
                printf("Секция: [%s]\n", current_section);
            } else {
                fprintf(stderr, "Ошибка: Некорректный формат секции: %s\n", line);
            }
        }
        // Если это пара ключ=значение
        else {
            char *delimiter = strchr(trimmed_line, '=');
            if (delimiter) {
                *delimiter = '\0';
                char *key = trim(trimmed_line);
                char *value = trim(delimiter + 1);
                printf("  Ключ: %s, Значение: %s (Секция: %s)\n", key, value, current_section);
            } else {
                fprintf(stderr, "Ошибка: Некорректная строка: %s\n", line);
            }
        }
    }

    fclose(file);
}

// Пример использования
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <ini-файл>\n", argv[0]);
        return 1;
    }

    parse_ini(argv[1]);
    return 0;
}
*/

#endif // !DISPATCH_SERVER_H