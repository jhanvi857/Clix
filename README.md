# Clix - A Custom Unix Shell

Clix is a lightweight, feature-rich shell implementation written in C. It provides a REPL environment with command parsing, execution, built-in commands, command history management, and intelligent fuzzy search capabilities.

## Features

### Core Features
- **Full Command Parsing** - Tokenizes and processes shell commands with support for arguments
- **Command Execution** - Executes both built-in and external system programs
- **I/O Redirection** - Supports input (`<`) and output (`>`) redirection
- **Background Execution** - Run processes in the background with the `&` operator
- **Built-in Commands** - Implements essential shell built-ins (`cd`, `echo`, `exit`, `help`, etc.)
- **Command History** - Persistent history management with save/load functionality

### Unique Features
- **Fuzzy Command Suggestions** - Suggests similar commands when misspelled (e.g., `lst` suggests `ls`)
- **History-based Search** - Search through previous commands with fuzzy matching
- **Smart Prompt** - Displays user@hostname:cwd format

---

## Architecture

### System Design

```
┌─────────────────────────────────────────────────────────────┐
│                         Main Loop                           │
│                    (REPL - main.c)                          │
│  - Print Prompt                                             │
│  - Read User Input                                          │
│  - Process & Execute Commands                              │
└─────────────────────────────────────────────────────────────┘
                              │
                ┌─────────────┼─────────────┐
                │             │             │
                ▼             ▼             ▼
        ┌────────────┐  ┌──────────┐  ┌────────────┐
        │  Parser    │  │ Builtins │  │  History   │
        │ (parser.c) │  │(built..c)│  │(history.c) │
        └────────────┘  └──────────┘  └────────────┘
                │             │             │
                └─────────────┼─────────────┘
                              │
                              ▼
                    ┌─────────────────────┐
                    │  Executor           │
                    │ (executor.c)        │
                    │                     │
                    │ - Fork & Execute    │
                    │ - I/O Redirection   │
                    │ - Background Tasks  │
                    └─────────────────────┘
                              │
                ┌─────────────┼─────────────┐
                │             │             │
                ▼             ▼             ▼
        ┌────────────┐  ┌──────────┐  ┌────────────┐
        │   Utils    │  │  Fuzzy   │  │System Calls│
        │(utils.c)   │  │(fuzzy.c) │  │ (execvp)   │
        └────────────┘  └──────────┘  └────────────┘
```

### Module Responsibility Map

| Module | File | Responsibility |
|--------|------|-----------------|
| **Parser** | `parser.c` / `parser.h` | Tokenizes input string into `Command` structure, handles argument parsing |
| **Executor** | `executor.c` / `executor.h` | Forks child processes, handles I/O redirection, manages background jobs |
| **Builtins** | `builtins.c` / `builtins.h` | Implements shell commands (`cd`, `echo`, `exit`, `help`, `history`) |
| **History** | `history.c` / `history.h` | Manages command history in memory, persists to file, retrieves past commands |
| **Fuzzy** | `fuzzy.c` / `fuzzy.h` | Provides fuzzy matching algorithm to suggest corrections for misspelled commands |
| **Utils** | `utils.c` / `utils.h` | String utilities, memory management, helper functions |
| **Shell** | `shell.h` | Defines core data structures (`Command` struct) and constants |

---

## Data Flow

### Command Execution Pipeline

```
User Input
    │
    ▼
parse_input() ──────► Command struct (args, argc, redirects, background flag)
    │
    ▼
history_add() ──────► Store in history buffer
    │
    ▼
execute(Command*) ──► Check if builtin?
    │                  │
    │                  ├─ YES: run_builtin() ──────► Execute (cd, echo, etc.)
    │                  │
    │                  └─ NO: fork() ──────► execvp() on child
    │
    └──────► Apply I/O redirects (< >)
    │
    ▼
[Command Complete]
    │
    ▼
Return to Prompt
```

### Fuzzy Suggestion Workflow

```
User types: "lst"
    │
    ▼
suggest_command("lst") ──► Fuzzy matching algorithm
    │
    ├─► Compare against known commands
    ├─► Calculate similarity score
    ├─► Find best match
    │
    ▼
Return: "Did you mean: ls?"
    │
    ▼
Display suggestion
```

---

## Command Structure

```c
typedef struct {
    char  *args[MAX_ARGS];      // Command arguments and flags
    int    argc;                 // Argument count
    char  *input_file;           // File for input redirection (<)
    char  *output_file;          // File for output redirection (>)
    int    append;               // Append mode flag (>>)
    int    background;           // Background execution flag (&)
} Command;
```

---

## Building and Running

### Build
```bash
make
```
This compiles all source files and creates the `shell` binary.

### Run
```bash
./shell
```

### Clean
```bash
make clean  # (if supported in your Makefile)
```

---

## Supported Built-in Commands

| Command | Usage | Description |
|---------|-------|-------------|
| `cd` | `cd <path>` | Change directory |
| `echo` | `echo <args>` | Print text |
| `exit` | `exit [code]` | Exit the shell |
| `help` | `help` | Show available commands |
| `history` | `history` | Display command history |

---

## Example Usage

```bash
$ pwd
/home/user/projects

$ cd /tmp
$ echo "Hello, Clix!"
Hello, Clix!

$ ls -la &              # Background execution
$ history               # View history
$ lst                   # Typo detected
Did you mean: ls?

$ cat file.txt > output.txt     # Output redirection
$ sort < input.txt              # Input redirection
```

---

## Project Statistics

- **Total Lines of Code**: ~1000+ (across all modules)
- **Number of Modules**: 7 core components
- **Build Tool**: GNU Make
- **Language**: C99
- **Dependencies**: Standard C Library (libc)

---

## Key Implementation Details

### Parser
- Handles whitespace tokenization
- Detects special operators (`<`, `>`, `&`)
- Builds argument vector for execution

### Executor
- Uses Unix `fork()` and `execvp()` system calls
- Manages parent-child process communication
- Implements redirection using file descriptors

### Fuzzy Matching
- Calculates string similarity scores
- Suggests closest matching commands
- Improves user experience with typo correction

### History
- In-memory circular buffer (up to 100 commands)
- Persistent file storage for history
- Searchable command history

---

## Development

### Adding a New Built-in Command
1. Add command name to `builtins.c`
2. Implement handler function
3. Register in `run_builtin()` switch statement

### Adding Features
Check the `include/` header files for existing APIs and extend as needed.

---

## License

This project is provided as-is for educational and research purposes.

---

Welcome to Clix. Happy coding!