#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "shell.h"

Command *parse_input(char *line) {
    Command *head = calloc(1, sizeof(Command));
    Command *curr = head;
    char *token = strtok(line, " \t");

    while (token) {
        if (strcmp(token, "|") == 0) {
            curr->next = calloc(1, sizeof(Command));
            curr = curr->next;
        } else if (strcmp(token, "<") == 0) {
            curr->input_file = strtok(NULL, " \t");
        } else if (strcmp(token, ">>") == 0) {
            curr->output_file = strtok(NULL, " \t");
            curr->append = 1;
        } else if (strcmp(token, ">") == 0) {
            curr->output_file = strtok(NULL, " \t");
        } else if (strcmp(token, "&") == 0) {
            // Background flag applies to the whole pipeline
            head->background = 1;
        } else {
            if (curr->argc < MAX_ARGS - 1) {
                if (token[0] == '$') {
                    char *val = getenv(token + 1);
                    curr->args[curr->argc++] = val ? strdup(val) : strdup("");
                } else {
                    curr->args[curr->argc++] = strdup(token);
                }
            }
        }
        token = strtok(NULL, " \t");
    }
    
    if (head->argc == 0) {
        free(head);
        return NULL;
    }
    return head;
}

void free_command(Command *cmd) {
    if (!cmd) return;
    free_command(cmd->next);
    for (int i = 0; i < cmd->argc; i++) free(cmd->args[i]);
    free(cmd);
}