#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "history.h"

#define HIST_FILE ".mysh_history"

static char *entries[MAX_HISTORY];
static int   count = 0;

void history_init(void) {
    FILE *f = fopen(HIST_FILE, "r");
    if (!f) return;
    char buf[MAX_INPUT];
    while (fgets(buf, sizeof(buf), f) && count < MAX_HISTORY) {
        buf[strcspn(buf, "\n")] = '\0';
        entries[count++] = strdup(buf);
    }
    fclose(f);
}

void history_add(const char *line) {
    if (count == MAX_HISTORY) { free(entries[0]); memmove(entries, entries+1, (MAX_HISTORY-1)*sizeof(char*)); count--; }
    entries[count++] = strdup(line);
}

void history_print(void) {
    for (int i = 0; i < count; i++) printf("  %3d  %s\n", i+1, entries[i]);
}

void history_save(void) {
    FILE *f = fopen(HIST_FILE, "w");
    if (!f) return;
    for (int i = 0; i < count; i++) fprintf(f, "%s\n", entries[i]);
    fclose(f);
}