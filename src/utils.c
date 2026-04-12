#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "shell.h"

void print_prompt(void) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    // Shorten $HOME to ~
    char *home = getenv("HOME");
    char  display[256];
    if (home && strncmp(cwd, home, strlen(home)) == 0)
        snprintf(display, sizeof(display), "~%s", cwd + strlen(home));
    else
        strncpy(display, cwd, sizeof(display));

    printf("\033[1;32m%s\033[0m:\033[1;34m%s\033[0m$ ", SHELL_NAME, display);
    fflush(stdout);
}