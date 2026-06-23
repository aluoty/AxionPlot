#ifndef FONT_H
#define FONT_H

#include "raylib.h"

Font UiLoadFont(int screen_height);
void UiUnloadFont(void);
Font UiGetFont(void);
int UiFontSize(int screen_height);

#endif
