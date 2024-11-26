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
		memset(start + padding + conetnt_length, background, padding);
		break;
	}
	}

	text[length] = '\0';
	return text;
}