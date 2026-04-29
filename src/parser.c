#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "shell.h"

static char *next_token(char **cursor) {
    char *start;
    char *end;
    size_t length;
    char *token;
    char quote = 0;

    while (**cursor && isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }

    if (**cursor == '\0') {
        return NULL;
    }

    if (**cursor == '>' || **cursor == '<' || **cursor == '|' || **cursor == '&') {
        if (**cursor == '>' && (*cursor)[1] == '>') {
            token = strdup(">>");
            (*cursor) += 2;
            return token;
        }

        token = (char *)malloc(2);
        if (!token) return NULL;
        token[0] = *(*cursor)++;
        token[1] = '\0';
        return token;
    }

    start = *cursor;
    end = *cursor;

    while (*end) {
        if (quote) {
            if (*end == quote) {
                quote = 0;
                end++;
                continue;
            }
            end++;
            continue;
        }

        if (*end == '"' || *end == '\'') {
            quote = *end++;
            continue;
        }

        if (isspace((unsigned char)*end) || *end == '>' || *end == '<' || *end == '|' || *end == '&') {
            break;
        }

        end++;
    }

    length = (size_t)(end - start);
    token = (char *)malloc(length + 1);
    if (!token) return NULL;

    size_t j = 0;
    quote = 0;
    for (char *p = start; p < end; p++) {
        if (quote) {
            if (*p == quote) {
                quote = 0;
                continue;
            }
            token[j++] = *p;
            continue;
        }

        if (*p == '"' || *p == '\'') {
            quote = *p;
            continue;
        }

        token[j++] = *p;
    }
    token[j] = '\0';
    *cursor = end;
    return token;
}

Command *parse_input(char *line) {
    Command *head = calloc(1, sizeof(Command));
    Command *curr = head;
    char *cursor = line;
    char *token = next_token(&cursor);

    while (token) {
        if (strcmp(token, "|") == 0) {
            curr->next = calloc(1, sizeof(Command));
            curr = curr->next;
        } else if (strcmp(token, "<") == 0) {
            free(curr->input_file);
            curr->input_file = next_token(&cursor);
        } else if (strcmp(token, ">>") == 0) {
            free(curr->output_file);
            curr->output_file = next_token(&cursor);
            curr->append = 1;
        } else if (strcmp(token, ">") == 0) {
            free(curr->output_file);
            curr->output_file = next_token(&cursor);
        } else if (strcmp(token, "&") == 0) {
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
        free(token);
        token = next_token(&cursor);
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
    free(cmd->input_file);
    free(cmd->output_file);
    free(cmd);
}