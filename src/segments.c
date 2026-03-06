#include "promptforge.h"

/* ── ユーザー名 ── */
void get_username(char *out, size_t size) {
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        strncpy(out, pw->pw_name, size - 1);
    } else {
        const char *user = getenv("USER");
        strncpy(out, user ? user : "user", size - 1);
    }
    out[size - 1] = '\0';
}

/* ── ホスト名 ── */
void get_hostname(char *out, size_t size) {
    if (gethostname(out, size) != 0) {
        strncpy(out, "localhost", size - 1);
    }
    out[size - 1] = '\0';
    /* FQDN → 短縮 */
    char *dot = strchr(out, '.');
    if (dot) *dot = '\0';
}

/* ── カレントディレクトリ ── */
void get_cwd(char *out, size_t size, int shorten) {
    char cwd[MAX_PATH_LEN];
    if (!getcwd(cwd, sizeof(cwd))) {
        strncpy(out, "?", size - 1);
        out[size - 1] = '\0';
        return;
    }

    /* ホームディレクトリを ~ に置換 */
    const char *home = getenv("HOME");
    char path[MAX_PATH_LEN];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(path, sizeof(path), "~%s", cwd + strlen(home));
    } else {
        strncpy(path, cwd, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    if (!shorten) {
        strncpy(out, path, size - 1);
        out[size - 1] = '\0';
        return;
    }

    /* 短縮: 最後のディレクトリのみ表示 */
    char *last = strrchr(path, '/');
    if (last && last != path) {
        strncpy(out, last + 1, size - 1);
    } else if (path[0] == '~' && path[1] == '\0') {
        strncpy(out, "~", size - 1);
    } else {
        strncpy(out, path, size - 1);
    }
    out[size - 1] = '\0';
}

/* ── Git ブランチ ── */
void get_git_branch(char *out, size_t size) {
    out[0] = '\0';

    /* .git/HEAD を探して上位ディレクトリも検索 */
    char cwd[MAX_PATH_LEN];
    if (!getcwd(cwd, sizeof(cwd))) return;

    char dir[MAX_PATH_LEN];
    strncpy(dir, cwd, sizeof(dir) - 1);

    while (1) {
        char head_path[MAX_PATH_LEN + 16];
        snprintf(head_path, sizeof(head_path), "%s/.git/HEAD", dir);

        FILE *fp = fopen(head_path, "r");
        if (fp) {
            char line[256];
            if (fgets(line, sizeof(line), fp)) {
                /* "ref: refs/heads/branchname" */
                if (strncmp(line, "ref: refs/heads/", 16) == 0) {
                    char *br = line + 16;
                    size_t len = strlen(br);
                    if (len > 0 && br[len-1] == '\n') br[len-1] = '\0';
                    strncpy(out, br, size - 1);
                } else {
                    /* detached HEAD: コミットハッシュ先頭7文字 */
                    line[7] = '\0';
                    strncpy(out, line, size - 1);
                }
            }
            fclose(fp);
            out[size - 1] = '\0';
            return;
        }

        /* 親ディレクトリへ */
        char *slash = strrchr(dir, '/');
        if (!slash || slash == dir) break;
        *slash = '\0';
    }
}

/* ── Git ステータス ── */
void get_git_status(char *out, size_t size) {
    out[0] = '\0';

    FILE *fp = popen("git status --porcelain 2>/dev/null", "r");
    if (!fp) return;

    int modified = 0, untracked = 0, staged = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '?' && line[1] == '?') untracked++;
        else if (line[0] != ' ' && line[0] != '\0' && line[0] != '\n') staged++;
        else if (line[1] == 'M' || line[1] == 'D') modified++;
    }
    pclose(fp);

    char tmp[64] = "";
    if (staged)    snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), "+%d ", staged);
    if (modified)  snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), "~%d ", modified);
    if (untracked) snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), "?%d ", untracked);

    /* 末尾スペース除去 */
    size_t l = strlen(tmp);
    if (l > 0 && tmp[l-1] == ' ') tmp[l-1] = '\0';

    strncpy(out, tmp, size - 1);
    out[size - 1] = '\0';
}

/* ── 時刻 ── */
void get_time(char *out, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(out, size, "%H:%M:%S", tm_info);
}

/* ── 日付 ── */
void get_date(char *out, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(out, size, "%Y-%m-%d", tm_info);
}

/* ── OS 名 ── */
void get_os(char *out, size_t size) {
#if defined(__APPLE__)
    strncpy(out, " macOS", size - 1);
#elif defined(__linux__)
    /* /etc/os-release から PRETTY_NAME を取得 */
    FILE *fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *val = line + 12;
                if (*val == '"') val++;
                size_t l = strlen(val);
                if (l > 0 && val[l-1] == '\n') val[--l] = '\0';
                if (l > 0 && val[l-1] == '"')  val[--l] = '\0';
                strncpy(out, val, size - 1);
                fclose(fp);
                out[size - 1] = '\0';
                return;
            }
        }
        fclose(fp);
    }
    strncpy(out, " Linux", size - 1);
#elif defined(__FreeBSD__)
    strncpy(out, " FreeBSD", size - 1);
#else
    strncpy(out, "OS", size - 1);
#endif
    out[size - 1] = '\0';
}

/* ── シェル ── */
void get_shell(char *out, size_t size) {
    const char *shell = getenv("SHELL");
    if (!shell) { strncpy(out, "sh", size - 1); out[size-1]='\0'; return; }
    const char *base = strrchr(shell, '/');
    strncpy(out, base ? base + 1 : shell, size - 1);
    out[size - 1] = '\0';
}

/* ── Python venv ── */
void get_python_venv(char *out, size_t size) {
    const char *venv = getenv("VIRTUAL_ENV");
    if (!venv || venv[0] == '\0') { out[0] = '\0'; return; }
    const char *base = strrchr(venv, '/');
    strncpy(out, base ? base + 1 : venv, size - 1);
    out[size - 1] = '\0';
}

/* ── Node.js バージョン ── */
void get_node_ver(char *out, size_t size) {
    out[0] = '\0';
    FILE *fp = popen("node --version 2>/dev/null", "r");
    if (!fp) return;
    char line[64];
    if (fgets(line, sizeof(line), fp)) {
        size_t l = strlen(line);
        if (l > 0 && line[l-1] == '\n') line[l-1] = '\0';
        strncpy(out, line, size - 1);
        out[size - 1] = '\0';
    }
    pclose(fp);
}

/* ── バッテリー (Linux) ── */
void get_battery(char *out, size_t size) {
    out[0] = '\0';
#ifdef __linux__
    /* /sys/class/power_supply/BAT0/ */
    const char *bat_paths[] = {
        "/sys/class/power_supply/BAT0",
        "/sys/class/power_supply/BAT1",
        NULL
    };
    for (int i = 0; bat_paths[i]; i++) {
        char cap_path[256], stat_path[256];
        snprintf(cap_path,  sizeof(cap_path),  "%s/capacity", bat_paths[i]);
        snprintf(stat_path, sizeof(stat_path), "%s/status",   bat_paths[i]);

        FILE *fp = fopen(cap_path, "r");
        if (!fp) continue;
        int cap = 0;
        fscanf(fp, "%d", &cap);
        fclose(fp);

        char status[32] = "Unknown";
        fp = fopen(stat_path, "r");
        if (fp) {
            fscanf(fp, "%31s", status);
            fclose(fp);
        }

        const char *icon = strcmp(status, "Charging") == 0 ? "⚡" : (cap > 20 ? "🔋" : "🪫");
        snprintf(out, size, "%s %d%%", icon, cap);
        return;
    }
#elif defined(__APPLE__)
    FILE *fp = popen("pmset -g batt 2>/dev/null | grep -o '[0-9]*%'", "r");
    if (fp) {
        char line[32];
        if (fgets(line, sizeof(line), fp)) {
            size_t l = strlen(line);
            if (l > 0 && line[l-1] == '\n') line[l-1] = '\0';
            snprintf(out, size, "🔋 %s", line);
        }
        pclose(fp);
    }
#endif
}

/* ── セグメント描画 ── */
void render_segment(const Segment *seg, int exit_code, char *out, size_t size) {
    char content[MAX_SEGMENT_LEN] = "";

    switch (seg->type) {
    case SEG_TEXT:
        strncpy(content, seg->text, sizeof(content) - 1);
        break;
    case SEG_USERNAME:
        get_username(content, sizeof(content));
        break;
    case SEG_HOSTNAME:
        get_hostname(content, sizeof(content));
        break;
    case SEG_CWD:
        get_cwd(content, sizeof(content), 0);
        break;
    case SEG_CWD_SHORT:
        get_cwd(content, sizeof(content), 1);
        break;
    case SEG_GIT_BRANCH: {
        char branch[256];
        get_git_branch(branch, sizeof(branch));
        if (branch[0]) snprintf(content, sizeof(content), " %s", branch);
        break;
    }
    case SEG_GIT_STATUS: {
        char status[128];
        get_git_status(status, sizeof(status));
        if (status[0]) strncpy(content, status, sizeof(content) - 1);
        break;
    }
    case SEG_TIME:
        get_time(content, sizeof(content));
        break;
    case SEG_DATE:
        get_date(content, sizeof(content));
        break;
    case SEG_EXIT_CODE:
        if (exit_code != 0) snprintf(content, sizeof(content), "✗ %d", exit_code);
        break;
    case SEG_SHELL:
        get_shell(content, sizeof(content));
        break;
    case SEG_OS:
        get_os(content, sizeof(content));
        break;
    case SEG_NEWLINE:
        strncpy(out, "\n", size - 1);
        out[size - 1] = '\0';
        return;
    case SEG_SPACE:
        strncpy(out, " ", size - 1);
        out[size - 1] = '\0';
        return;
    case SEG_ENV:
        if (seg->text[0]) {
            const char *val = getenv(seg->text);
            if (val) strncpy(content, val, sizeof(content) - 1);
        }
        break;
    case SEG_PYTHON_VENV:
        get_python_venv(content, sizeof(content));
        if (content[0]) {
            char tmp[MAX_SEGMENT_LEN];
            snprintf(tmp, sizeof(tmp), "🐍 %s", content);
            strncpy(content, tmp, sizeof(content) - 1);
        }
        break;
    case SEG_NODE_VER:
        get_node_ver(content, sizeof(content));
        if (content[0]) {
            char tmp[MAX_SEGMENT_LEN];
            snprintf(tmp, sizeof(tmp), "⬡ %s", content);
            strncpy(content, tmp, sizeof(content) - 1);
        }
        break;
    case SEG_BATTERY:
        get_battery(content, sizeof(content));
        break;
    case SEG_SEPARATOR:
        strncpy(content, seg->sep_char[0] ? seg->sep_char : "│", sizeof(content) - 1);
        break;
    default:
        break;
    }

    content[sizeof(content) - 1] = '\0';

    /* 空コンテンツは描画しない */
    if (content[0] == '\0') {
        out[0] = '\0';
        return;
    }

    /* スタイル適用 */
    char style_str[256] = "";
    style_apply(&seg->style, style_str, sizeof(style_str));

    snprintf(out, size, "%s%s%s%s%s%s",
             style_str,
             seg->prefix,
             content,
             seg->suffix,
             RESET,
             "");
}
