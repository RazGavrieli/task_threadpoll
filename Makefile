.PHONY: all
all: coder task stdinExample

coder: codec.h stdin_main.c
	gcc stdin_main.c -pthread -L. -l Codec -o coder

task: codec.h stdin_main.c
	gcc stdin_main.c -pthread -L. -l Codec -o encoder

stdinExample: codec.h stdin_main.c
	gcc stdin_main.c -pthread -L. -l Codec -o tester

.PHONY: clean
clean:
	-rm coder encoder tester 2>/dev/null
