#ifndef UI_H
#define UI_H

#include <string.h>

#define TA_HORIZONTAL_LEFT 0x01
#define TA_HORIZONTAL_RIGHT 0x02
#define TA_HORIZONTAL_CENTER 0x03

char* horizontal_align_text(char* text, size_t length, int background, int align);

#endif // !UI_H