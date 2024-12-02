#ifndef UI_H
#define UI_H

#include <string.h>

#pragma warning(disable: 4996)

#define TA_HORIZONTAL_LEFT 0x01
#define TA_HORIZONTAL_RIGHT 0x02
#define TA_HORIZONTAL_CENTER 0x03

char* horizontal_align_text(char* text, size_t length, int background, int align);

char* trimWord(const char* text);
char* trim(char* str);
char* find_substr(const char* str, const char* substr);

typedef struct _char_column {
	size_t width;
	int background;
	int align;
} char_column;

typedef struct _char_table {
	size_t columns_count;
	char_column* columns;
	size_t lines_count;
	const char*** array;
} char_table;

char* draw_table_line(char* text, const char** line, char_column* columns_width, size_t columns_count);
char* draw_table(char_table table, char* text);

#endif // !UI_H