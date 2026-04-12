CC = gcc
CFLAGS = -Iinclude

SRC = src/main.c src/executor.c src/parser.c src/builtins.c src/utils.c src/history.c src/fuzzy.c

all:
	$(CC) $(CFLAGS) $(SRC) -o shell