#ifndef PROMPTFORGE_H
#define PROMPTFORGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>

/* ── ANSI エスケープ ── */
#define ESC "\x1b"
#define RESET       ESC "[0m"
#define BOLD        ESC "[1m"
#define DIM         ESC "[2m"
#define ITALIC      ESC "[3m"
#define UNDERLINE   ESC "[4m"
#define BLINK       ESC "[5m"
#define REVERSE     ESC "[7m"
#define STRIKE      ESC "[9m"

/* 前景色 (標準) */
#define FG_BLACK    ESC "[30m"
#define FG_RED      ESC "[31m"
#define FG_GREEN    ESC "[32m"
#define FG_YELLOW   ESC "[33m"
#define FG_BLUE     ESC "[34m"
#define FG_MAGENTA  ESC "[35m"
#define FG_CYAN     ESC "[36m"
#define FG_WHITE    ESC "[37m"

/* 前景色 (明るい) */
#define FG_BRIGHT_BLACK    ESC "[90m"
#define FG_BRIGHT_RED      ESC "[91m"
#define FG_BRIGHT_GREEN    ESC "[92m"
#define FG_BRIGHT_YELLOW   ESC "[93m"
#define FG_BRIGHT_BLUE     ESC "[94m"
#define FG_BRIGHT_MAGENTA  ESC "[95m"
#define FG_BRIGHT_CYAN     ESC "[96m"
#define FG_BRIGHT_WHITE    ESC "[97m"

/* 背景色 (標準) */
#define BG_BLACK    ESC "[40m"
#define BG_RED      ESC "[41m"
#define BG_GREEN    ESC "[42m"
#define BG_YELLOW   ESC "[43m"
#define BG_BLUE     ESC "[44m"
#define BG_MAGENTA  ESC "[45m"
#define BG_CYAN     ESC "[46m"
#define BG_WHITE    ESC "[47m"

/* 背景色 (明るい) */
#define BG_BRIGHT_BLACK    ESC "[100m"
#define BG_BRIGHT_RED      ESC "[101m"
#define BG_BRIGHT_GREEN    ESC "[102m"
#define BG_BRIGHT_YELLOW   ESC "[103m"
#define BG_BRIGHT_BLUE     ESC "[104m"
#define BG_BRIGHT_MAGENTA  ESC "[105m"
#define BG_BRIGHT_CYAN     ESC "[106m"
#define BG_BRIGHT_WHITE    ESC "[107m"

/* RGB カラー */
#define FG_RGB(r,g,b)  ESC "[38;2;" #r ";" #g ";" #b "m"
#define BG_RGB(r,g,b)  ESC "[48;2;" #r ";" #g ";" #b "m"

/* ── バッファサイズ ── */
#define MAX_PROMPT_LEN   4096
#define MAX_SEGMENT_LEN  512
#define MAX_PATH_LEN     PATH_MAX
#define MAX_LINE_LEN     1024
#define MAX_CONFIG_LEN   8192
#define MAX_CMD_LEN      256

/* ── セグメントタイプ ── */
typedef enum {
    SEG_TEXT = 0,
    SEG_USERNAME,
    SEG_HOSTNAME,
    SEG_CWD,
    SEG_CWD_SHORT,
    SEG_GIT_BRANCH,
    SEG_GIT_STATUS,
    SEG_TIME,
    SEG_DATE,
    SEG_EXIT_CODE,
    SEG_SHELL,
    SEG_OS,
    SEG_JOBS,
    SEG_NEWLINE,
    SEG_SPACE,
    SEG_ENV,
    SEG_CMD_DURATION,
    SEG_BATTERY,
    SEG_PYTHON_VENV,
    SEG_NODE_VER,
    SEG_SEPARATOR,
    SEG_POWERLINE_RIGHT,
    SEG_POWERLINE_LEFT,
    SEG_UNKNOWN,
    SEG_COUNT
} SegmentType;

/* ── スタイル ── */
typedef struct {
    char fg[64];       /* 前景色エスケープ */
    char bg[64];       /* 背景色エスケープ */
    int  bold;
    int  italic;
    int  underline;
    int  dim;
    int  reverse;
} Style;

/* ── セグメント ── */
typedef struct {
    SegmentType type;
    char        text[MAX_SEGMENT_LEN];  /* SEG_TEXT / SEG_ENV 変数名など */
    Style       style;
    int         enabled;
    char        prefix[64];
    char        suffix[64];
    int         powerline;              /* powerlineスタイル区切り */
    char        sep_char[16];          /* カスタム区切り文字 */
} Segment;

/* ── プロンプト設定 ── */
typedef struct {
    Segment segments[64];
    int     segment_count;
    char    prompt_char[16];           /* 最後のプロンプト文字 ($ / # 等) */
    Style   prompt_char_style;
    Style   prompt_char_error_style;
    int     newline_before_prompt;
    int     right_prompt;              /* 右プロンプト有効 */
    Segment right_segments[32];
    int     right_segment_count;
} PromptConfig;

/* ── 関数プロトタイプ ── */

/* color.c */
void   style_apply(const Style *s, char *buf, size_t size);
void   style_reset(char *buf, size_t size);
int    parse_color(const char *name, char *out, size_t size, int is_bg);
void   rgb_fg(int r, int g, int b, char *out, size_t size);
void   rgb_bg(int r, int g, int b, char *out, size_t size);

/* segments.c */
void   render_segment(const Segment *seg, int exit_code, char *out, size_t size);
void   get_username(char *out, size_t size);
void   get_hostname(char *out, size_t size);
void   get_cwd(char *out, size_t size, int shorten);
void   get_git_branch(char *out, size_t size);
void   get_git_status(char *out, size_t size);
void   get_time(char *out, size_t size);
void   get_date(char *out, size_t size);
void   get_os(char *out, size_t size);
void   get_shell(char *out, size_t size);
void   get_python_venv(char *out, size_t size);
void   get_node_ver(char *out, size_t size);
void   get_battery(char *out, size_t size);

/* config.c */
int    load_config(const char *path, PromptConfig *cfg);
int    save_default_config(const char *path);
void   init_default_config(PromptConfig *cfg);
void   print_config_help(void);
char  *get_config_path(char *buf, size_t size);

/* prompt.c */
void   render_prompt(const PromptConfig *cfg, int exit_code, char *out, size_t size);
void   render_right_prompt(const PromptConfig *cfg, int exit_code, char *out, size_t size);

/* init.c */
void   print_shell_init(const char *shell, const char *binary_path);

#endif /* PROMPTFORGE_H */
