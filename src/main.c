#include "promptforge.h"

static void print_usage(const char *prog) {
    printf(
        BOLD FG_BRIGHT_CYAN
        "╔═══════════════════════════════════╗\n"
        "║      PromptForge  v1.0.0          ║\n"
        "║  シンプル・高機能プロンプト設定   ║\n"
        "╚═══════════════════════════════════╝\n"
        RESET "\n"
        BOLD "使い方:\n" RESET
        "  %s prompt [--exit-code N] [--config PATH]\n"
        "  %s rprompt [--exit-code N] [--config PATH]\n"
        "  %s init <bash|zsh|fish>\n"
        "  %s config-path\n"
        "  %s print-config\n"
        "  %s help\n"
        "\n"
        BOLD "クイックスタート:\n" RESET
        "  1. eval \"$(promptforge init bash)\"   # または zsh/fish\n"
        "  2. promptforge config-path           # 設定ファイルの場所を確認\n"
        "  3. nano $(promptforge config-path)   # 設定を編集\n"
        "\n",
        prog, prog, prog, prog, prog, prog
    );
}

/* ── プレビュー: ターミナルでサンプル表示 ── */
static void print_preview(const PromptConfig *cfg) {
    char prompt[MAX_PROMPT_LEN];
    /* exit_code=0 の場合 */
    render_prompt(cfg, 0, prompt, sizeof(prompt));
    printf(FG_BRIGHT_BLACK "--- プレビュー (exit=0) ---\n" RESET);
    printf("%s\n", prompt);

    /* exit_code=1 の場合 */
    render_prompt(cfg, 1, prompt, sizeof(prompt));
    printf(FG_BRIGHT_BLACK "--- プレビュー (exit=1) ---\n" RESET);
    printf("%s\n", prompt);
}

/* ── 現在の設定をテキストで出力 ── */
static void print_current_config(const PromptConfig *cfg) {
    printf(BOLD "セグメント数: " RESET "%d\n", cfg->segment_count);
    for (int i = 0; i < cfg->segment_count; i++) {
        const Segment *s = &cfg->segments[i];
        if (!s->enabled) continue;

        const char *type_names[] = {
            "text","username","hostname","cwd","cwd_short",
            "git_branch","git_status","time","date","exit_code",
            "shell","os","jobs","newline","space","env",
            "cmd_duration","battery","python_venv","node_version",
            "separator","powerline_right","powerline_left","unknown"
        };
        const char *tname = (s->type < SEG_COUNT) ? type_names[s->type] : "?";
        printf("  [%d] type=%-14s fg=%s  bg=%s  bold=%d\n",
               i, tname, s->style.fg[0] ? "(set)" : "-",
               s->style.bg[0] ? "(set)" : "-", s->style.bold);
    }
    printf(BOLD "プロンプト文字: " RESET "%s\n", cfg->prompt_char);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    const char *cmd = argv[1];

    /* ── init ── */
    if (strcmp(cmd, "init") == 0) {
        const char *shell = (argc >= 3) ? argv[2] : "bash";
        print_shell_init(shell, argv[0]);
        return 0;
    }

    /* ── config-path ── */
    if (strcmp(cmd, "config-path") == 0) {
        char path[MAX_PATH_LEN];
        printf("%s\n", get_config_path(path, sizeof(path)));
        return 0;
    }

    /* ── help ── */
    if (strcmp(cmd, "help") == 0) {
        print_config_help();
        return 0;
    }

    /* ── 設定読み込み ── */
    PromptConfig cfg;
    char config_path[MAX_PATH_LEN];
    get_config_path(config_path, sizeof(config_path));

    /* --config オプション検索 */
    for (int i = 2; i < argc - 1; i++) {
        if (strcmp(argv[i], "--config") == 0) {
            strncpy(config_path, argv[i+1], sizeof(config_path) - 1);
            break;
        }
    }

    if (!load_config(config_path, &cfg)) {
        /* 設定ファイルが無ければデフォルトを生成 */
        init_default_config(&cfg);
        save_default_config(config_path);
    }

    /* ── print-config ── */
    if (strcmp(cmd, "print-config") == 0) {
        print_current_config(&cfg);
        return 0;
    }

    /* ── preview ── */
    if (strcmp(cmd, "preview") == 0) {
        print_preview(&cfg);
        return 0;
    }

    /* ── exit-code 取得 ── */
    int exit_code = 0;
    for (int i = 2; i < argc - 1; i++) {
        if (strcmp(argv[i], "--exit-code") == 0) {
            exit_code = atoi(argv[i+1]);
            break;
        }
    }

    /* ── prompt ── */
    if (strcmp(cmd, "prompt") == 0) {
        char out[MAX_PROMPT_LEN];
        render_prompt(&cfg, exit_code, out, sizeof(out));
        printf("%s", out);
        return 0;
    }

    /* ── rprompt ── */
    if (strcmp(cmd, "rprompt") == 0) {
        char out[MAX_PROMPT_LEN];
        render_right_prompt(&cfg, exit_code, out, sizeof(out));
        if (out[0]) printf("%s", out);
        return 0;
    }

    fprintf(stderr, "不明なコマンド: %s\n", cmd);
    print_usage(argv[0]);
    return 1;
}
