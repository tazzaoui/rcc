CC=gcc
CFLAGS=-Wall -Werror -std=gnu99 -pedantic -g
LFLAGS=-lm
TESTS=test
EXE=rcc $(TESTS) 

all: $(EXE) 

rcc: list.o utils.o pairs.o r.o x.o c.o rcc.o main.o
	$(CC) main.o r.o x.o c.o rcc.o pairs.o utils.o list.o $(LFLAGS) -o rcc

test: list.o utils.o pairs.o r.o x.o c.o rcc.o main_tests.o tests.o 
	$(CC) main_tests.o tests.o pairs.o r.o x.o c.o rcc.o utils.o list.o $(LFLAGS) -o test

main.o: main.c
	$(CC) -c $(CFLAGS) main.c -o main.o

main_tests.o: tests/main.c
	$(CC) -c $(CFLAGS) tests/main.c -o main_tests.o

tests.o: tests/tests.c
	$(CC) -c $(CFLAGS) tests/tests.c -o tests.o

rcc.o: src/rcc.c src/rcc.h
	$(CC) -c $(CFLAGS) src/rcc.c -o rcc.o

r.o: src/r.c src/r.h
	$(CC) -c $(CFLAGS) src/r.c -o r.o

x.o: src/x.c src/x.h
	$(CC) -c $(CFLAGS) src/x.c -o x.o

c.o: src/c.c src/c.h
	$(CC) -c $(CFLAGS) src/c.c -o c.o

utils.o: src/utils.c src/utils.h
	$(CC) -c $(CFLAGS) src/utils.c -o utils.o

pairs.o: src/pairs.c src/pairs.h
	$(CC) -c $(CFLAGS) src/pairs.c -o pairs.o

list.o: src/list.c src/list.h
	$(CC) -c $(CFLAGS) src/list.c -o list.o

clean:
	rm -f *.o *~ a.out *.s *.bin $(EXE) 

format:
	find . -name "*.c" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
	find . -name "*.h" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
