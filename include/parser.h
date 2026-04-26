#pragma once
#include "shell.h"   // needs Command type

Command *parse_input(char *line);
void     free_command(Command *cmd);