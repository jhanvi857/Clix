#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"
#include "history.h"

int run_builtin(Command *cmd) {
    if (!cmd->args[0]) return 0;

    if (strcmp(cmd->args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    if (strcmp(cmd->args[0], "cd") == 0) {
        const char *dir = cmd->args[1] ? cmd->args[1] : getenv("HOME");
        if (chdir(dir) != 0) perror("cd");
        return 1;
    }
    if (strcmp(cmd->args[0], "history") == 0) {
        history_print(); return 1;
    }
    if (strcmp(cmd->args[0], "help") == 0) {
        printf("Built-in commands: cd, exit, history, help\n");
        printf("Features: I/O redirect (<, >, >>), background (&), typo correction\n");
        return 1;
    }
    return 0;
}