#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "fuzzy.h"

static int edit_distance(const char *a, const char *b) {
    int la = strlen(a), lb = strlen(b);
    int dp[la + 1][lb + 1];

    for (int i = 0; i <= la; i++) dp[i][0] = i;
    for (int j = 0; j <= lb; j++) dp[0][j] = j;

    for (int i = 1; i <= la; i++)
        for (int j = 1; j <= lb; j++) {
            int cost = (a[i-1] != b[j-1]);
            int del  = dp[i-1][j] + 1;
            int ins  = dp[i][j-1] + 1;
            int sub  = dp[i-1][j-1] + cost;
            dp[i][j] = del < ins ? (del < sub ? del : sub)
                                 : (ins < sub ? ins : sub);
        }
    return dp[la][lb];
}

char *suggest_command(const char *wrong_cmd) {
    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char path_copy[4096];
    strncpy(path_copy, path_env, sizeof(path_copy) - 1);

    char   *best      = NULL;
    int     best_dist = 3;   
    char   *dir       = strtok(path_copy, ":");

    while (dir) {
        DIR *dp = opendir(dir);
        if (dp) {
            struct dirent *entry;
            while ((entry = readdir(dp))) {
                int d = edit_distance(wrong_cmd, entry->d_name);
                if (d < best_dist) {
                    best_dist = d;
                    free(best);
                    best = strdup(entry->d_name);
                }
            }
            closedir(dp);
        }
        dir = strtok(NULL, ":");
    }
    return best;   
}