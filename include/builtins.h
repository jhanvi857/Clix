#pragma once
#include "shell.h"

int run_builtin(Command *cmd);  // returns 1 if handled, 0 if not