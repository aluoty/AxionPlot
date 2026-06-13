#ifndef WEB_API_H
#define WEB_API_H

#include "display.h"

void WebApiBindDisplay(DisplaySettings *display);
void WebApiSyncCanvas(int width, int height, int quality_preset);

#endif
