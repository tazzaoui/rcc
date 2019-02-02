CC=gcc
CFLAGS=-Wall -Werror -std=c99 -pedantic -g
TESTS=test
EXE=rcc $(TESTS) 

all: $(EXE) 

rcc: list.o utils.o rcc.o main.o
	$(CC) main.o rcc.o utils.o list.o -o rcc

test: list.o utils.o rcc.o main_tests.o tests.o 
	$(CC) main_tests.o tests.o rcc.o utils.o list.o -o test

main.o: main.c
	$(CC) -c $(CFLAGS) main.c -o main.o

main_tests.o: tests/main.c
	$(CC) -c $(CFLAGS) tests/main.c -o main_tests.o

tests.o: tests/tests.c
	$(CC) -c $(CFLAGS) tests/tests.c -o tests.o

rcc.o: src/rcc.c src/rcc.h
	$(CC) -c $(CFLAGS) src/rcc.c -o rcc.o

utils.o: src/utils.c src/utils.h
	$(CC) -c $(CFLAGS) src/utils.c -o utils.o

list.o: src/list.c src/list.h
	$(CC) -c $(CFLAGS) src/list.c -o list.o

clean:
	rm -f *.o *~ $(EXE) 

format:
	clang-format -i -style=google main.c src/*.h src/*.h tests/*.c tests/*.h 
