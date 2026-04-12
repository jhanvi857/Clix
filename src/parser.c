#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "shell.h"

Command parse_input(char *line) {
    Command cmd = {0};
    char *token = strtok(line, " \t");

    while (token && cmd.argc < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            cmd.input_file = strtok(NULL, " \t");
        } else if (strcmp(token, ">>") == 0) {
            cmd.output_file = strtok(NULL, " \t");
            cmd.append = 1;
        } else if (strcmp(token, ">") == 0) {
            cmd.output_file = strtok(NULL, " \t");
        } else if (strcmp(token, "&") == 0) {
            cmd.background = 1;
        } else {
            cmd.args[cmd.argc++] = token;
        }
        token = strtok(NULL, " \t");
    }
    cmd.args[cmd.argc] = NULL;
    return cmd;
}