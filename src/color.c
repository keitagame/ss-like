#include "promptforge.h"

/* 色名 → エスケープシーケンス変換 */
typedef struct { const char *name; const char *fg; const char *bg; } ColorEntry;

static const ColorEntry COLOR_TABLE[] = {
    {"black",          FG_BLACK,         BG_BLACK},
    {"red",            FG_RED,           BG_RED},
    {"green",          FG_GREEN,         BG_GREEN},
    {"yellow",         FG_YELLOW,        BG_YELLOW},
    {"blue",           FG_BLUE,          BG_BLUE},
    {"magenta",        FG_MAGENTA,       BG_MAGENTA},
    {"cyan",           FG_CYAN,          BG_CYAN},
    {"white",          FG_WHITE,         BG_WHITE},
    {"bright_black",   FG_BRIGHT_BLACK,  BG_BRIGHT_BLACK},
    {"bright_red",     FG_BRIGHT_RED,    BG_BRIGHT_RED},
    {"bright_green",   FG_BRIGHT_GREEN,  BG_BRIGHT_GREEN},
    {"bright_yellow",  FG_BRIGHT_YELLOW, BG_BRIGHT_YELLOW},
    {"bright_blue",    FG_BRIGHT_BLUE,   BG_BRIGHT_BLUE},
    {"bright_magenta", FG_BRIGHT_MAGENTA,BG_BRIGHT_MAGENTA},
    {"bright_cyan",    FG_BRIGHT_CYAN,   BG_BRIGHT_CYAN},
    {"bright_white",   FG_BRIGHT_WHITE,  BG_BRIGHT_WHITE},
    {"gray",           FG_BRIGHT_BLACK,  BG_BRIGHT_BLACK},
    {"orange",         ESC "[38;5;214m", ESC "[48;5;214m"},
    {"pink",           ESC "[38;5;213m", ESC "[48;5;213m"},
    {"purple",         ESC "[38;5;135m", ESC "[48;5;135m"},
    {"lime",           ESC "[38;5;154m", ESC "[48;5;154m"},
    {"teal",           ESC "[38;5;30m",  ESC "[48;5;30m"},
    {"gold",           ESC "[38;5;220m", ESC "[48;5;220m"},
    {"salmon",         ESC "[38;5;210m", ESC "[48;5;210m"},
    {"lavender",       ESC "[38;5;147m", ESC "[48;5;147m"},
    {"none",           "",               ""},
    {NULL, NULL, NULL}
};

/* RGB 前景色 */
void rgb_fg(int r, int g, int b, char *out, size_t size) {
    snprintf(out, size, ESC "[38;2;%d;%d;%dm", r, g, b);
}

/* RGB 背景色 */
void rgb_bg(int r, int g, int b, char *out, size_t size) {
    snprintf(out, size, ESC "[48;2;%d;%d;%dm", r, g, b);
}

/* hex (#rrggbb) → RGB 解析 */
static int parse_hex_color(const char *hex, int *r, int *g, int *b) {
    if (hex[0] == '#') hex++;
    if (strlen(hex) != 6) return 0;
    char rr[3] = {hex[0], hex[1], 0};
    char gg[3] = {hex[2], hex[3], 0};
    char bb[3] = {hex[4], hex[5], 0};
    *r = (int)strtol(rr, NULL, 16);
    *g = (int)strtol(gg, NULL, 16);
    *b = (int)strtol(bb, NULL, 16);
    return 1;
}

/* "rgb(r,g,b)" 形式を解析 */
static int parse_rgb_func(const char *s, int *r, int *g, int *b) {
    return sscanf(s, "rgb(%d,%d,%d)", r, g, b) == 3 ||
           sscanf(s, "rgb(%d, %d, %d)", r, g, b) == 3;
}

/* 色文字列をエスケープシーケンスに変換 */
int parse_color(const char *name, char *out, size_t size, int is_bg) {
    if (!name || name[0] == '\0') {
        out[0] = '\0';
        return 1;
    }

    /* 名前テーブル検索 */
    for (int i = 0; COLOR_TABLE[i].name; i++) {
        if (strcasecmp(name, COLOR_TABLE[i].name) == 0) {
            strncpy(out, is_bg ? COLOR_TABLE[i].bg : COLOR_TABLE[i].fg, size - 1);
            out[size - 1] = '\0';
            return 1;
        }
    }

    /* #rrggbb 形式 */
    if (name[0] == '#') {
        int r, g, b;
        if (parse_hex_color(name, &r, &g, &b)) {
            if (is_bg) rgb_bg(r, g, b, out, size);
            else       rgb_fg(r, g, b, out, size);
            return 1;
        }
    }

    /* rgb() 形式 */
    int r, g, b;
    if (parse_rgb_func(name, &r, &g, &b)) {
        if (is_bg) rgb_bg(r, g, b, out, size);
        else       rgb_fg(r, g, b, out, size);
        return 1;
    }

    /* 256色 (数値) */
    char *end;
    long idx = strtol(name, &end, 10);
    if (*end == '\0' && idx >= 0 && idx <= 255) {
        if (is_bg) snprintf(out, size, ESC "[48;5;%ldm", idx);
        else       snprintf(out, size, ESC "[38;5;%ldm", idx);
        return 1;
    }

    out[0] = '\0';
    return 0;
}

/* スタイルをバッファに書き出す */
void style_apply(const Style *s, char *buf, size_t size) {
    buf[0] = '\0';
    size_t used = 0;

    if (s->bg[0]) {
        strncat(buf, s->bg, size - used - 1);
        used = strlen(buf);
    }
    if (s->fg[0]) {
        strncat(buf, s->fg, size - used - 1);
        used = strlen(buf);
    }
    if (s->bold)      { strncat(buf, BOLD,      size - used - 1); used = strlen(buf); }
    if (s->italic)    { strncat(buf, ITALIC,    size - used - 1); used = strlen(buf); }
    if (s->underline) { strncat(buf, UNDERLINE, size - used - 1); used = strlen(buf); }
    if (s->dim)       { strncat(buf, DIM,       size - used - 1); used = strlen(buf); }
    if (s->reverse)   { strncat(buf, REVERSE,   size - used - 1); used = strlen(buf); }
    (void)used;
}

/* リセット */
void style_reset(char *buf, size_t size) {
    strncpy(buf, RESET, size - 1);
    buf[size - 1] = '\0';
}
