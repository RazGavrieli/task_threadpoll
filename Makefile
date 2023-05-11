.PHONY: all
all: coder task stdinExample

coder: codec.h stdin_main.c
	gcc stdin_main.c -L. -l Codec -o coder

task: codec.h stdin_main.c
	gcc stdin_main.c -L. -l Codec -o encoder

stdinExample: codec.h stdin_main.c
	gcc stdin_main.c -L. -l Codec -o tester

.PHONY: clean
clean:
	-rm encoder tester 2>/dev/null
