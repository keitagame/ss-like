#include "promptforge.h"

/* ── デフォルト設定の初期化 ── */
void init_default_config(PromptConfig *cfg) {
    memset(cfg, 0, sizeof(*cfg));

    int i = 0;

    /* セグメント0: username@hostname (背景付き) */
    cfg->segments[i].type = SEG_USERNAME;
    cfg->segments[i].enabled = 1;
    parse_color("bright_blue",  cfg->segments[i].style.fg, 64, 0);
    parse_color("none",         cfg->segments[i].style.bg, 64, 1);
    cfg->segments[i].style.bold = 1;
    strncpy(cfg->segments[i].prefix, "", 63);
    strncpy(cfg->segments[i].suffix, "", 63);
    i++;

    /* セグメント1: @ 区切り */
    cfg->segments[i].type = SEG_TEXT;
    strncpy(cfg->segments[i].text, "@", MAX_SEGMENT_LEN - 1);
    parse_color("bright_black", cfg->segments[i].style.fg, 64, 0);
    cfg->segments[i].enabled = 1;
    i++;

    /* セグメント2: hostname */
    cfg->segments[i].type = SEG_HOSTNAME;
    cfg->segments[i].enabled = 1;
    parse_color("cyan",  cfg->segments[i].style.fg, 64, 0);
    cfg->segments[i].style.bold = 1;
    i++;

    /* セグメント3: スペース */
    cfg->segments[i].type = SEG_SPACE;
    cfg->segments[i].enabled = 1;
    i++;

    /* セグメント4: CWD (背景色付き) */
    cfg->segments[i].type = SEG_CWD;
    cfg->segments[i].enabled = 1;
    parse_color("#1e3a5f", cfg->segments[i].style.bg, 64, 1);
    parse_color("bright_white", cfg->segments[i].style.fg, 64, 0);
    cfg->segments[i].style.bold = 1;
    strncpy(cfg->segments[i].prefix, " 📁 ", 63);
    strncpy(cfg->segments[i].suffix, " ", 63);
    i++;

    /* セグメント5: スペース */
    cfg->segments[i].type = SEG_SPACE;
    cfg->segments[i].enabled = 1;
    i++;

    /* セグメント6: git ブランチ */
    cfg->segments[i].type = SEG_GIT_BRANCH;
    cfg->segments[i].enabled = 1;
    parse_color("#2d4a1e", cfg->segments[i].style.bg, 64, 1);
    parse_color("bright_green", cfg->segments[i].style.fg, 64, 0);
    cfg->segments[i].style.bold = 1;
    strncpy(cfg->segments[i].suffix, " ", 63);
    i++;

    /* セグメント7: git status */
    cfg->segments[i].type = SEG_GIT_STATUS;
    cfg->segments[i].enabled = 1;
    parse_color("bright_yellow", cfg->segments[i].style.fg, 64, 0);
    strncpy(cfg->segments[i].prefix, "[", 63);
    strncpy(cfg->segments[i].suffix, "]", 63);
    i++;

    /* セグメント8: 改行 */
    cfg->segments[i].type = SEG_NEWLINE;
    cfg->segments[i].enabled = 1;
    i++;

    /* セグメント9: exit code (エラー時のみ表示) */
    cfg->segments[i].type = SEG_EXIT_CODE;
    cfg->segments[i].enabled = 1;
    parse_color("bright_red", cfg->segments[i].style.fg, 64, 0);
    cfg->segments[i].style.bold = 1;
    strncpy(cfg->segments[i].suffix, " ", 63);
    i++;

    cfg->segment_count = i;

    /* プロンプト文字 */
    strncpy(cfg->prompt_char, "❯", 15);
    parse_color("bright_green",  cfg->prompt_char_style.fg, 64, 0);
    cfg->prompt_char_style.bold = 1;
    parse_color("bright_red",    cfg->prompt_char_error_style.fg, 64, 0);
    cfg->prompt_char_error_style.bold = 1;

    cfg->newline_before_prompt = 0;
}

/* ── 設定ファイルパス取得 ── */
char *get_config_path(char *buf, size_t size) {
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    snprintf(buf, size, "%s/.config/promptforge/config.pf", home);
    return buf;
}

/* ── 文字列トリム ── */
static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    size_t l = strlen(s);
    while (l > 0 && isspace((unsigned char)s[l-1])) s[--l] = '\0';
    return s;
}

/* ── スタイルキー解析 ── */
static void parse_style_key(const char *key, const char *val, Style *st) {
    if      (strcmp(key, "fg")        == 0) parse_color(val, st->fg, 64, 0);
    else if (strcmp(key, "bg")        == 0) parse_color(val, st->bg, 64, 1);
    else if (strcmp(key, "bold")      == 0) st->bold      = atoi(val);
    else if (strcmp(key, "italic")    == 0) st->italic    = atoi(val);
    else if (strcmp(key, "underline") == 0) st->underline = atoi(val);
    else if (strcmp(key, "dim")       == 0) st->dim       = atoi(val);
    else if (strcmp(key, "reverse")   == 0) st->reverse   = atoi(val);
}

/* セグメント名 → タイプ */
static SegmentType parse_segment_type(const char *name) {
    if      (strcmp(name, "text")         == 0) return SEG_TEXT;
    else if (strcmp(name, "username")     == 0) return SEG_USERNAME;
    else if (strcmp(name, "hostname")     == 0) return SEG_HOSTNAME;
    else if (strcmp(name, "cwd")          == 0) return SEG_CWD;
    else if (strcmp(name, "cwd_short")    == 0) return SEG_CWD_SHORT;
    else if (strcmp(name, "git_branch")   == 0) return SEG_GIT_BRANCH;
    else if (strcmp(name, "git_status")   == 0) return SEG_GIT_STATUS;
    else if (strcmp(name, "time")         == 0) return SEG_TIME;
    else if (strcmp(name, "date")         == 0) return SEG_DATE;
    else if (strcmp(name, "exit_code")    == 0) return SEG_EXIT_CODE;
    else if (strcmp(name, "shell")        == 0) return SEG_SHELL;
    else if (strcmp(name, "os")           == 0) return SEG_OS;
    else if (strcmp(name, "jobs")         == 0) return SEG_JOBS;
    else if (strcmp(name, "newline")      == 0) return SEG_NEWLINE;
    else if (strcmp(name, "space")        == 0) return SEG_SPACE;
    else if (strcmp(name, "env")          == 0) return SEG_ENV;
    else if (strcmp(name, "python_venv")  == 0) return SEG_PYTHON_VENV;
    else if (strcmp(name, "node_version") == 0) return SEG_NODE_VER;
    else if (strcmp(name, "battery")      == 0) return SEG_BATTERY;
    else if (strcmp(name, "separator")    == 0) return SEG_SEPARATOR;
    return SEG_UNKNOWN;
}

/*
 * ── 設定ファイル形式 ──
 *
 * [segment]
 * type = cwd
 * fg = bright_white
 * bg = #1e3a5f
 * bold = 1
 * prefix =  📁 
 * suffix =  
 * enabled = 1
 *
 * [prompt]
 * char = ❯
 * char_fg = bright_green
 * char_error_fg = bright_red
 * newline = 0
 */
int load_config(const char *path, PromptConfig *cfg) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    init_default_config(cfg);
    cfg->segment_count = 0;
    cfg->right_segment_count = 0;

    char line[MAX_LINE_LEN];
    Segment *cur_seg = NULL;
    int in_prompt = 0;
    int in_right = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *l = trim(line);
        if (l[0] == '#' || l[0] == '\0') continue;

        /* セクション開始 */
        if (l[0] == '[') {
            in_prompt = 0;
            cur_seg = NULL;

            if (strncmp(l, "[segment]", 9) == 0) {
                if (cfg->segment_count < 64) {
                    cur_seg = &cfg->segments[cfg->segment_count++];
                    memset(cur_seg, 0, sizeof(Segment));
                    cur_seg->enabled = 1;
                    in_right = 0;
                }
            } else if (strncmp(l, "[right_segment]", 15) == 0) {
                if (cfg->right_segment_count < 32) {
                    cur_seg = &cfg->right_segments[cfg->right_segment_count++];
                    memset(cur_seg, 0, sizeof(Segment));
                    cur_seg->enabled = 1;
                    in_right = 1;
                }
            } else if (strncmp(l, "[prompt]", 8) == 0) {
                in_prompt = 1;
            }
            continue;
        }

        /* key = value */
        char *eq = strchr(l, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = trim(l);
        char *val = trim(eq + 1);

        if (in_prompt) {
            if      (strcmp(key, "char")          == 0) strncpy(cfg->prompt_char, val, 15);
            else if (strcmp(key, "char_fg")        == 0) parse_color(val, cfg->prompt_char_style.fg, 64, 0);
            else if (strcmp(key, "char_error_fg")  == 0) parse_color(val, cfg->prompt_char_error_style.fg, 64, 0);
            else if (strcmp(key, "newline")        == 0) cfg->newline_before_prompt = atoi(val);
            else if (strcmp(key, "right_prompt")   == 0) cfg->right_prompt = atoi(val);
        } else if (cur_seg) {
            if      (strcmp(key, "type")    == 0) cur_seg->type = parse_segment_type(val);
            else if (strcmp(key, "text")    == 0) strncpy(cur_seg->text, val, MAX_SEGMENT_LEN - 1);
            else if (strcmp(key, "enabled") == 0) cur_seg->enabled = atoi(val);
            else if (strcmp(key, "prefix")  == 0) strncpy(cur_seg->prefix, val, 63);
            else if (strcmp(key, "suffix")  == 0) strncpy(cur_seg->suffix, val, 63);
            else if (strcmp(key, "sep")     == 0) strncpy(cur_seg->sep_char, val, 15);
            else parse_style_key(key, val, &cur_seg->style);
        }
    }

    fclose(fp);
    return 1;
}

/* ── デフォルト設定ファイル生成 ── */
int save_default_config(const char *path) {
    /* ディレクトリ作成 */
    char dir[MAX_PATH_LEN];
    strncpy(dir, path, sizeof(dir) - 1);
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        char cmd[MAX_PATH_LEN + 16];
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir);
        system(cmd);
    }

    FILE *fp = fopen(path, "w");
    if (!fp) return 0;

    fprintf(fp,
"# PromptForge 設定ファイル\n"
"# ~/.config/promptforge/config.pf\n"
"#\n"
"# [segment] / [right_segment] ブロックを並べてプロンプトを構築します\n"
"# type: username, hostname, cwd, cwd_short, git_branch, git_status,\n"
"#       time, date, exit_code, shell, os, python_venv, node_version,\n"
"#       battery, text, env, separator, newline, space\n"
"# fg/bg: 色名 (red, bright_blue, cyan...) または #rrggbb または rgb(r,g,b) または 0-255\n"
"# bold/italic/underline/dim/reverse: 0 or 1\n"
"\n"
"[segment]\n"
"type    = username\n"
"fg      = bright_blue\n"
"bold    = 1\n"
"\n"
"[segment]\n"
"type    = text\n"
"text    = @\n"
"fg      = bright_black\n"
"\n"
"[segment]\n"
"type    = hostname\n"
"fg      = cyan\n"
"bold    = 1\n"
"\n"
"[segment]\n"
"type    = space\n"
"\n"
"[segment]\n"
"type    = cwd\n"
"fg      = bright_white\n"
"bg      = #1e3a5f\n"
"bold    = 1\n"
"prefix  =  📁 \n"
"suffix  =  \n"
"\n"
"[segment]\n"
"type    = space\n"
"\n"
"[segment]\n"
"type    = git_branch\n"
"fg      = bright_green\n"
"bg      = #2d4a1e\n"
"bold    = 1\n"
"suffix  =  \n"
"\n"
"[segment]\n"
"type    = git_status\n"
"fg      = bright_yellow\n"
"prefix  = [\n"
"suffix  = ]\n"
"\n"
"[segment]\n"
"type    = newline\n"
"\n"
"[segment]\n"
"type    = exit_code\n"
"fg      = bright_red\n"
"bold    = 1\n"
"suffix  =  \n"
"\n"
"[prompt]\n"
"char          = ❯\n"
"char_fg       = bright_green\n"
"char_error_fg = bright_red\n"
"newline       = 0\n"
"right_prompt  = 0\n"
    );

    fclose(fp);
    return 1;
}

/* ── ヘルプ ── */
void print_config_help(void) {
    printf(
        BOLD FG_BRIGHT_CYAN "PromptForge 設定リファレンス\n" RESET
        "\n"
        BOLD "セグメントタイプ:\n" RESET
        "  username      " FG_BRIGHT_BLACK "ユーザー名\n" RESET
        "  hostname      " FG_BRIGHT_BLACK "ホスト名\n" RESET
        "  cwd           " FG_BRIGHT_BLACK "カレントディレクトリ (フルパス)\n" RESET
        "  cwd_short     " FG_BRIGHT_BLACK "カレントディレクトリ (末尾のみ)\n" RESET
        "  git_branch    " FG_BRIGHT_BLACK "Gitブランチ名\n" RESET
        "  git_status    " FG_BRIGHT_BLACK "Gitステータス (+staged ~modified ?untracked)\n" RESET
        "  time          " FG_BRIGHT_BLACK "現在時刻 (HH:MM:SS)\n" RESET
        "  date          " FG_BRIGHT_BLACK "今日の日付 (YYYY-MM-DD)\n" RESET
        "  exit_code     " FG_BRIGHT_BLACK "直前コマンドの終了コード (非0の場合のみ)\n" RESET
        "  shell         " FG_BRIGHT_BLACK "シェル名\n" RESET
        "  os            " FG_BRIGHT_BLACK "OS名\n" RESET
        "  python_venv   " FG_BRIGHT_BLACK "Python仮想環境名\n" RESET
        "  node_version  " FG_BRIGHT_BLACK "Node.jsバージョン\n" RESET
        "  battery       " FG_BRIGHT_BLACK "バッテリー残量\n" RESET
        "  text          " FG_BRIGHT_BLACK "固定テキスト (text= で指定)\n" RESET
        "  env           " FG_BRIGHT_BLACK "環境変数 (text= で変数名を指定)\n" RESET
        "  separator     " FG_BRIGHT_BLACK "区切り文字 (sep= でカスタム可)\n" RESET
        "  newline       " FG_BRIGHT_BLACK "改行\n" RESET
        "  space         " FG_BRIGHT_BLACK "スペース\n" RESET
        "\n"
        BOLD "色指定:\n" RESET
        "  black, red, green, yellow, blue, magenta, cyan, white\n"
        "  bright_black, bright_red, ... bright_white\n"
        "  orange, pink, purple, lime, teal, gold, lavender\n"
        "  #rrggbb  例: #ff6b35\n"
        "  rgb(r,g,b)  例: rgb(255,107,53)\n"
        "  0-255  (256色)\n"
        "\n"
        BOLD "スタイル属性:\n" RESET
        "  bold=1, italic=1, underline=1, dim=1, reverse=1\n"
        "\n"
        BOLD "プロンプトオプション:\n" RESET
        "  [prompt]\n"
        "  char = ❯         # プロンプト文字\n"
        "  char_fg = bright_green\n"
        "  char_error_fg = bright_red\n"
        "  newline = 0       # 1にするとプロンプト前に空行\n"
        "\n"
    );
}
