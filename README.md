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

        # Clix - A Custom Shell

        Clix is a compact shell written in C. It starts a read-eval-print loop, parses one line at a time, stores command history, runs built-ins, executes external programs, and suggests close matches for misspelled commands.

        ## What The Project Does

        Clix is organized around a single `Command` structure defined in [include/shell.h](include/shell.h). Every line you type is turned into that structure by the parser, recorded by history, and then passed to the executor.

        The project currently supports:

        - built-ins: `cd`, `pwd`, `echo`, `exit`, `history`, `help`
        - external commands via `execvp`
        - input and output redirection: `<`, `>`, `>>`
        - background execution with `&`
        - fuzzy command suggestions for typos
        - persistent history saved to `.mysh_history`

        On Unix-like systems, external commands use process spawning and pipes where needed. On Windows, the code includes a simpler fallback path, and the README should be read as describing the core shell behavior rather than a full POSIX implementation.

        ## Code Walkthrough

        ### `main.c`: The REPL

        [`src/main.c`](src/main.c) contains the main loop. It disables `SIGINT`, initializes history, prints the prompt, reads a line with `fgets`, trims the newline, stores the input in history, parses it, executes it, and then frees the command structure.

        ### `parser.c`: Turns Text Into a Command

        [`src/parser.c`](src/parser.c) converts the raw input line into a `Command` object. It splits the line into tokens, recognizes operators like `|`, `<`, `>`, `>>`, and `&`, expands `$VAR`-style environment references, and fills in:

        - `args[]` and `argc`
        - `input_file`
        - `output_file`
        - `append`
        - `background`

        ### `executor.c`: Runs the Command

        [`src/executor.c`](src/executor.c) decides whether to run a built-in or launch an external program. It also handles redirection by wiring file descriptors before execution. For built-ins, the shell can still apply redirection so commands like `echo hello >> file1.txt` write to a file instead of the terminal.

        ### `builtins.c`: Shell Commands

        [`src/builtins.c`](src/builtins.c) implements the built-ins directly in-process:

        - `cd` changes the current working directory
        - `pwd` prints the current directory
        - `echo` prints its arguments
        - `history` prints stored commands
        - `help` shows supported commands
        - `exit` terminates the shell

        ### `history.c`: Persistent History

        [`src/history.c`](src/history.c) keeps a fixed-size history buffer in memory and saves it to `.mysh_history` when the shell exits. On startup it reloads the file so previous commands remain available.

        ### `fuzzy.c`: Command Suggestions

        [`src/fuzzy.c`](src/fuzzy.c) scans executable names from `PATH` and compares them with the typo using edit distance and prefix similarity. If the match is close enough, it prints a suggestion like `Did you mean: ls?`.

        ### `utils.c`: Prompt Rendering

        [`src/utils.c`](src/utils.c) prints the prompt in `user@hostname:cwd$` form and shortens the home directory to `~` when possible.

        ## Execution Flow

        ```mermaid
        flowchart TD
            A[User types a command] --> B[main.c reads line]
            B --> C[history_add stores raw input]
            C --> D[parse_input in parser.c]
            D --> E{Command valid?}
            E -- no --> Z[Ignore line / return to prompt]
            E -- yes --> F[execute in executor.c]
            F --> G{Builtin?}
            G -- yes --> H[run_builtin in builtins.c]
            G -- no --> I[spawn external command]
            H --> J[Apply redirection if needed]
            I --> K[Setup redirection / pipes]
            J --> L[Return to prompt]
            K --> L
        ```

        ## Module Map

        ```mermaid
        flowchart LR
            main[main.c] --> parser[parser.c]
            main --> history[history.c]
            main --> executor[executor.c]
            parser --> shell[shell.h Command struct]
            executor --> builtins[builtins.c]
            executor --> fuzzy[fuzzy.c]
            utils[utils.c] --> main
            utils --> executor
            history --> builtins
        ```

        ## Command Structure

        ```c
        typedef struct Command {
            char           *args[MAX_ARGS];
            int             argc;
            char           *input_file;
            char           *output_file;
            int             append;
            int             background;
            struct Command *next;
        } Command;
        ```
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
| **Shell** | `shell.h` | Defines core data structures (`Command` struct) and constants |

---

## Data Flow

### Command Execution Pipeline

```
User Input
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

### Fuzzy Suggestion Workflow

    │
    ├─► Calculate similarity score
    ├─► Find best match
    │
    ▼
Display suggestion
```
## Command Structure

    int    argc;                 // Argument count
    char  *input_file;           // File for input redirection (<)
} Command;

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