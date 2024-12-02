#include "./ui.h"

char* horizontal_align_text(char* text, size_t length, int background, int align) {
	size_t conetnt_length = strlen(text);
	size_t padding = 0;
	char* start = text;

	switch (align) {
	case TA_HORIZONTAL_LEFT: {
		padding = length - conetnt_length;

		memset(start + conetnt_length, background, padding);
		break;
	}
	case TA_HORIZONTAL_RIGHT: {
		padding = length - conetnt_length;

		memcpy(start + padding, text, conetnt_length);
		memset(start, background, padding);
		break;
	}
	case TA_HORIZONTAL_CENTER: {
		padding = (length - conetnt_length) >> 1;

		memcpy(start + padding, text, conetnt_length);
		memset(start, background, padding);
		memset(start + padding + conetnt_length, background, padding + ((length - conetnt_length) & 1));
		break;
	}
	}

	text[length] = '\0';
	return text;
}

char* trimWord(char* text) {
	size_t length = strlen(text);

	char* word = text;
	for (word = text; word < word + length; word++) {
		if (*word == ' ' || *word == '\0') {
			*(word++) = '\0';
			break;
		}
	}

	return word;
}

char* trim(char* str) {
	char* end;

	while (isspace((unsigned char)*str)) str++;

	if (*str == 0)
		return str;

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;

	*(end + 1) = '\0';

	return str;
}

char* find_substr(const char* str, const char* substr) {
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == substr[0]) {
			int j = 0;
			size_t substr_length = strlen(substr);
			for (; j < substr_length; j++) {
				if (str[i + j] != substr[j])
					break;
			}

			if (++j == substr) {
				return str + i;
			}
		}
	}
}

char* draw_table_line(char* text, const char** line, char_column* columns_width, size_t columns_count) {
	size_t width = 0;
	char buffer[64];

	for (size_t i = 0; i < columns_count; i++) {
		char_column column = columns_width[i];

		memcpy(buffer, line[i], strlen(line[i]) + 1);
		strcat(text, horizontal_align_text(buffer, column.width, column.background, column.align));
		width += column.width;
	}
	*(text += width) = '\n';

	return ++text;
}

char* draw_table(char_table table, char* text) {
	char* base_text = text;
	text = draw_table_line(text, table.array[0], table.columns, table.columns_count);
	size_t width = (text - 1) - base_text;

	memset(text, 205, width);
	*(text += width) = '\n';
	text++;

	for (size_t i = 1; i < table.lines_count; i++) {
		text = draw_table_line(text, table.array[i], table.columns, table.columns_count);
	}
	*(++text) = '\0';

	return text;
}