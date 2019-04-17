CC=gcc
CFLAGS=-Wall -Werror -std=gnu99 -pedantic -g
LFLAGS=-lm
OBJ=list.o utils.o pairs.o r.o x.o c.o rco.o rcc.o main.o 

rcc: $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o rcc

main.o: main.c
	$(CC) -c $(CFLAGS) main.c -o main.o

rcc.o: src/rcc.c src/rcc.h
	$(CC) -c $(CFLAGS) src/rcc.c -o rcc.o

rco.o: src/rco.c src/rco.h
	$(CC) -c $(CFLAGS) src/rco.c -o rco.o

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
	rm -f *.o *~ a.out *.s *.bin rcc
	cd tests && make clean

format:
	find . -name "*.c" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
	find . -name "*.h" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
	cd tests && make format
