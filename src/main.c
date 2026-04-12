#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "parser.h"
#include "executor.h"
#include "history.h"
#include "utils.h"
int main() {
    char input[MAX_INPUT];
    history_init();
    while(1) {
        print_prompt();
        if (!fgets(input, MAX_INPUT, stdin)) {
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;
        history_add(input);
        Command cmd = parse_input(input);
        if(cmd.argc==0) continue;
        execute(&cmd);
    }
    history_save();
    return 0;
}