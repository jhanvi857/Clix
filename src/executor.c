#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef _WIN32
#include <sys/wait.h>
#else
#include <process.h>
#include <io.h>
typedef int pid_t;
#define fork() (-1)
#define pipe(fds) (-1)
#endif
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
    if (!cmd) return;

    // Single built-in (no pipes)
    if (!cmd->next && run_builtin(cmd)) return;

#ifdef _WIN32
    // Windows Fallback: Single command execution without pipes/redirects
    if (!cmd->next && !cmd->input_file && !cmd->output_file && !cmd->background) {
        intptr_t status = _spawnvp(_P_WAIT, cmd->args[0], (const char * const *)cmd->args);
        if (status == -1) {
            fprintf(stderr, "%s: command not found or execution failed\n", cmd->args[0]);
            char *suggestion = suggest_command(cmd->args[0]);
            if (suggestion) {
                fprintf(stderr, "  Did you mean: \033[1;33m%s\033[0m ?\n", suggestion);
                free(suggestion);
            }
        }
        return;
    }
    // If it's more complex (pipes/redirects), warn the user
    if (cmd->next || cmd->input_file || cmd->output_file || cmd->background) {
        fprintf(stderr, "Pipes, redirection, and background tasks are not supported natively on Windows in this shell.\n");
        fprintf(stderr, "Please use MSYS2 or WSL for full Unix functionality.\n");
        return;
    }
#endif

    int num_cmds = 0;
    for (Command *c = cmd; c; c = c->next) num_cmds++;

    int pipefds[2 * (num_cmds - 1)];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) { perror("pipe"); return; }
    }

    Command *curr = cmd;
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++) close(pipefds[j]);

            setup_redirects(curr);

            if (run_builtin(curr)) exit(0);

            execvp(curr->args[0], (const char * const *)curr->args);
            fprintf(stderr, "%s: command not found\n", curr->args[0]);
            char *suggestion = suggest_command(curr->args[0]);
            if (suggestion) {
                fprintf(stderr, "  Did you mean: \033[1;33m%s\033[0m ?\n", suggestion);
                free(suggestion);
            }
            exit(127);
        } else if (pids[i] < 0) {
            perror("fork");
            return;
        }
        curr = curr->next;
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++) close(pipefds[i]);

    if (!cmd->background) {
        for (int i = 0; i < num_cmds; i++) {
#ifndef _WIN32
            waitpid(pids[i], NULL, 0);
#else
            // On Windows, native fork() is not available in standard GCC.
            // This part would require CreateProcess logic for full functionality.
#endif
        }
    } else {
        printf("[bg] pipeline started\n");
    }
}