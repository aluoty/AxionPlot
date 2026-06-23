#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Font g_font = {0};
static bool g_loaded = false;

static const char *FontSearchPaths(void) {
    const char *env = getenv("AXION_FONT");
    if (env && env[0]) return env;

    static const char *paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
        NULL
    };

    for (int i = 0; paths[i]; i++) {
        FILE *f = fopen(paths[i], "rb");
        if (f) {
            fclose(f);
            return paths[i];
        }
    }

    return NULL;
}

Font UiLoadFont(int screen_height) {
    if (g_loaded) return g_font;

    int size = UiFontSize(screen_height);
    const char *path = FontSearchPaths();

    if (path) {
        g_font = LoadFontEx(path, size, 0, 0);
        if (g_font.texture.id > 0) {
            SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
            g_loaded = true;
            return g_font;
        }
    }

    g_font = GetFontDefault();
    g_loaded = true;
    return g_font;
}

void UiUnloadFont(void) {
    if (g_loaded && g_font.texture.id > 0) {
        Font def = GetFontDefault();
        if (g_font.texture.id != def.texture.id) {
            UnloadFont(g_font);
        }
    }
    g_loaded = false;
}

Font UiGetFont(void) {
    return g_font;
}

int UiFontSize(int screen_height) {
    int size = (int)(screen_height * 0.018f);
    if (size < 14) size = 14;
    if (size > 22) size = 22;
    return size;
}
