#include "font.h"

#include "font_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Font g_font = {0};
static bool g_loaded = false;

Font UiLoadFont(int screen_height) {
    if (g_loaded) return g_font;

    int size = UiFontSize(screen_height);
    bool loaded = false;

#if !defined(__EMSCRIPTEN__)
    const char *env = getenv("AXION_FONT");
    if (env && env[0]) {
        g_font = LoadFontEx(env, size, 0, 0);
        if (g_font.texture.id > 0) loaded = true;
    }

    if (!loaded) {
        static const char *paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            NULL
        };
        for (int i = 0; paths[i]; i++) {
            g_font = LoadFontEx(paths[i], size, 0, 0);
            if (g_font.texture.id > 0) { loaded = true; break; }
        }
    }
#endif

    if (!loaded) {
        g_font = LoadFontFromMemory(
            ".ttf",
            _nix_store_37c8di1dc9zp4xfb1pzqdg1gbpbkniw5_dejavu_fonts_2_37_share_fonts_truetype_DejaVuSans_ttf,
            (int)_nix_store_37c8di1dc9zp4xfb1pzqdg1gbpbkniw5_dejavu_fonts_2_37_share_fonts_truetype_DejaVuSans_ttf_len,
            size, 0, 0
        );
    }

    if (g_font.texture.id > 0) {
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        g_font = GetFontDefault();
    }

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
