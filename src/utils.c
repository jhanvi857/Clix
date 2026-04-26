#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
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

    char hostname[256];
    char *user = getenv("USER");
    if (!user) user = getenv("USERNAME"); // Windows fallback

#ifdef _WIN32
    DWORD size = sizeof(hostname);
    if (!GetComputerNameA(hostname, &size)) strncpy(hostname, "windows", sizeof(hostname));
#else
    if (gethostname(hostname, sizeof(hostname)) != 0) strncpy(hostname, "unknown", sizeof(hostname));
#endif

    printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ", user ? user : "user", hostname, display);
    fflush(stdout);
}