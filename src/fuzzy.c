#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include "fuzzy.h"

static int min3(int a, int b, int c) {
    int m = (a < b) ? a : b;
    return (m < c) ? m : c;
}

static int edit_distance(const char *a, const char *b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    int dp[la + 1][lb + 1];

    for (int i = 0; i <= la; i++) dp[i][0] = i;
    for (int j = 0; j <= lb; j++) dp[0][j] = j;

    for (int i = 1; i <= la; i++) {
        for (int j = 1; j <= lb; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = min3(
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            );

            // Damerau transposition: handles swaps like "hepl" -> "help", "gti" -> "git".
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1]) {
                int trans = dp[i - 2][j - 2] + 1;
                if (trans < dp[i][j]) dp[i][j] = trans;
            }
        }
    }
    return dp[la][lb];
}

static int common_prefix_len(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return i;
}

static int preferred_rank(const char *cmd) {
    static const char *preferred[] = {
        "ls", "git", "cd", "help", "history", "exit", "pwd", "echo", "cat", "grep",
        "find", "make", "head", "tail", "cp", "mv", "rm", "mkdir", "touch", "clear",
        NULL
    };

    for (int i = 0; preferred[i]; i++) {
        if (strcmp(cmd, preferred[i]) == 0) return i;
    }
    return INT_MAX;
}

static void consider_candidate(const char *wrong_cmd, const char *candidate,
                               char **best, int *best_dist, int *best_pref,
                               int *best_rank, int *best_len) {
    if (!candidate || candidate[0] == '\0') return;
    if (strcmp(candidate, wrong_cmd) == 0) return;

    int d = edit_distance(wrong_cmd, candidate);
    int p = common_prefix_len(wrong_cmd, candidate);
    int r = preferred_rank(candidate);
    int l = (int)strlen(candidate);

    if (d < *best_dist ||
        (d == *best_dist && p > *best_pref) ||
        (d == *best_dist && p == *best_pref && r < *best_rank) ||
        (d == *best_dist && p == *best_pref && r == *best_rank && l < *best_len) ||
        (d == *best_dist && p == *best_pref && r == *best_rank && l == *best_len && *best && strcmp(candidate, *best) < 0)) {
        *best_dist = d;
        *best_pref = p;
        *best_rank = r;
        *best_len  = l;
        free(*best);
        *best = strdup(candidate);
    }
}

char *suggest_command(const char *wrong_cmd) {
    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    static const char *builtins[] = { "cd", "exit", "history", "help", NULL };

    char path_copy[4096];
    strncpy(path_copy, path_env, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char   *best      = NULL;
    int     best_dist = INT_MAX;
    int     best_pref = -1;
    int     best_rank = INT_MAX;
    int     best_len  = INT_MAX;
    char   *dir       = strtok(path_copy, ":");

    for (int i = 0; builtins[i]; i++) {
        consider_candidate(wrong_cmd, builtins[i], &best, &best_dist, &best_pref, &best_rank, &best_len);
    }

    while (dir) {
        DIR *dp = opendir(dir);
        if (dp) {
            struct dirent *entry;
            while ((entry = readdir(dp))) {
                if (entry->d_name[0] == '.') continue;

                char full[PATH_MAX];
                int n = snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
                if (n <= 0 || n >= (int)sizeof(full)) continue;
                if (access(full, X_OK) != 0) continue;

                consider_candidate(wrong_cmd, entry->d_name, &best, &best_dist, &best_pref, &best_rank, &best_len);
            }
            closedir(dp);
        }
        dir = strtok(NULL, ":");
    }

    if (best_dist > 2) {
        free(best);
        return NULL;
    }
    return best;
}