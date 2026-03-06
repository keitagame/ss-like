#include "promptforge.h"

/* ── プロンプト描画 ── */
void render_prompt(const PromptConfig *cfg, int exit_code, char *out, size_t size) {
    char buf[MAX_PROMPT_LEN] = "";
    size_t used = 0;

    if (cfg->newline_before_prompt) {
        strncat(buf, "\n", size - used - 1);
        used = strlen(buf);
    }

    /* セグメントを順に描画 */
    for (int i = 0; i < cfg->segment_count; i++) {
        const Segment *seg = &cfg->segments[i];
        if (!seg->enabled) continue;

        char seg_buf[MAX_SEGMENT_LEN * 2] = "";
        render_segment(seg, exit_code, seg_buf, sizeof(seg_buf));
        if (seg_buf[0]) {
            strncat(buf, seg_buf, size - used - 1);
            used = strlen(buf);
        }
    }

    /* プロンプト文字 */
    const Style *pstyle = (exit_code != 0) ? &cfg->prompt_char_error_style
                                            : &cfg->prompt_char_style;
    char style_str[256] = "";
    style_apply(pstyle, style_str, sizeof(style_str));

    char prompt_char_buf[128];
    snprintf(prompt_char_buf, sizeof(prompt_char_buf), "%s%s%s ",
             style_str, cfg->prompt_char, RESET);

    strncat(buf, prompt_char_buf, size - used - 1);

    strncpy(out, buf, size - 1);
    out[size - 1] = '\0';
}

/* ── 右プロンプト描画 ── */
void render_right_prompt(const PromptConfig *cfg, int exit_code, char *out, size_t size) {
    if (!cfg->right_prompt || cfg->right_segment_count == 0) {
        out[0] = '\0';
        return;
    }

    char buf[MAX_PROMPT_LEN] = "";
    size_t used = 0;

    for (int i = 0; i < cfg->right_segment_count; i++) {
        const Segment *seg = &cfg->right_segments[i];
        if (!seg->enabled) continue;

        char seg_buf[MAX_SEGMENT_LEN * 2] = "";
        render_segment(seg, exit_code, seg_buf, sizeof(seg_buf));
        if (seg_buf[0]) {
            strncat(buf, seg_buf, size - used - 1);
            used = strlen(buf);
        }
    }

    strncpy(out, buf, size - 1);
    out[size - 1] = '\0';
}
