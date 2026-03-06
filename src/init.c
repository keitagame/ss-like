#include "promptforge.h"

void print_shell_init(const char *shell, const char *binary_path) {
    if (!shell || !binary_path) {
        fprintf(stderr, "Usage: promptforge init <shell> [binary_path]\n");
        return;
    }

    if (strcmp(shell, "bash") == 0) {
        printf(
            "# PromptForge init (bash)\n"
            "# ~/.bashrc に以下を追加してください:\n"
            "#   eval \"$(promptforge init bash)\"\n"
            "\n"
            "__promptforge_precmd() {\n"
            "    local exit_code=$?\n"
            "    PS1=\"$(\"%s\" prompt --exit-code $exit_code 2>/dev/null)\"\n"
            "}\n"
            "PROMPT_COMMAND='__promptforge_precmd'\n",
            binary_path
        );
    } else if (strcmp(shell, "zsh") == 0) {
        printf(
            "# PromptForge init (zsh)\n"
            "# ~/.zshrc に以下を追加してください:\n"
            "#   eval \"$(promptforge init zsh)\"\n"
            "\n"
            "autoload -Uz add-zsh-hook\n"
            "\n"
            "__promptforge_precmd() {\n"
            "    local exit_code=$?\n"
            "    PROMPT=\"$(\"%s\" prompt --exit-code $exit_code 2>/dev/null)\"\n"
            "}\n"
            "\n"
            "add-zsh-hook precmd __promptforge_precmd\n"
            "\n"
            "# 右プロンプト有効化\n"
            "__promptforge_rprompt() {\n"
            "    local exit_code=$?\n"
            "    RPROMPT=\"$(\"%s\" rprompt --exit-code $exit_code 2>/dev/null)\"\n"
            "}\n"
            "add-zsh-hook precmd __promptforge_rprompt\n",
            binary_path, binary_path
        );
    } else if (strcmp(shell, "fish") == 0) {
        printf(
            "# PromptForge init (fish)\n"
            "# ~/.config/fish/config.fish に以下を追加してください:\n"
            "#   promptforge init fish | source\n"
            "\n"
            "function fish_prompt\n"
            "    set -l exit_code $status\n"
            "    \"%s\" prompt --exit-code $exit_code\n"
            "end\n"
            "\n"
            "function fish_right_prompt\n"
            "    set -l exit_code $status\n"
            "    \"%s\" rprompt --exit-code $exit_code\n"
            "end\n",
            binary_path, binary_path
        );
    } else {
        fprintf(stderr,
            "サポートされているシェル: bash, zsh, fish\n"
            "例: promptforge init bash\n"
        );
    }
}
