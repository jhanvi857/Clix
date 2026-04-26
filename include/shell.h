#pragma once
#define MAX_INPUT    1024
#define MAX_ARGS     64
#define MAX_HISTORY  100
#define SHELL_NAME   "Clix"

typedef struct Command {
    char           *args[MAX_ARGS];
    int             argc;
    char           *input_file;   
    char           *output_file;  
    int             append;       
    int             background;   
    struct Command *next;
} Command;