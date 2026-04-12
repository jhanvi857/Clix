#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "executor.h"
#include "builtins.h"
#include "fuzzy.h"

static void setup_redirects(Command *cmd) {
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) { perror("open"); exit(1); }
        dup2(fd, STDIN_FILENO); close(fd);
    }
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) { perror("open"); exit(1); }
        dup2(fd, STDOUT_FILENO); close(fd);
    }
}

void execute(Command *cmd) {
    // Try builtins first
    if (run_builtin(cmd)) return;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }

    if (pid == 0) {
        // Child
        setup_redirects(cmd);
        execvp(cmd->args[0], cmd->args);

        // execvp failed → command not found
        fprintf(stderr, "%s: command not found\n", cmd->args[0]);

        char *suggestion = suggest_command(cmd->args[0]);
        if (suggestion) {
            fprintf(stderr, "  Did you mean: \033[1;33m%s\033[0m ?\n", suggestion);
            free(suggestion);
        }
        exit(127);
    } else {
        // Parent
        if (!cmd->background)
            waitpid(pid, NULL, 0);
        else
            printf("[bg] pid %d\n", pid);
    }
}