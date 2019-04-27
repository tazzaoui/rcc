CC=gcc
CFLAGS=-Wall -Werror -std=gnu99 -pedantic -g
LFLAGS=-lm
EXE=test_passes test_r test_indiv
OBJ=../list.o ../utils.o ../pairs.o ../r.o ../x.o ../c.o ../rco.o ../econ.o ../live.o ../rcc.o tests.o

all: rcc $(EXE) 

test_passes: $(OBJ) test_passes.o
	$(CC) $(OBJ) test_passes.o -o test_passes $(LFLAGS)

test_r: $(OBJ) test_r.o
	$(CC) $(OBJ) test_r.o -o test_r $(LFLAGS)

test_indiv: $(OBJ) test_indiv.o
	$(CC) $(OBJ) test_indiv.o -o test_indiv $(LFLAGS)

test_passes.o: test_passes.c
	$(CC) -c $(CFLAGS) test_passes.c -o test_passes.o

test_r.o: test_r.c
	$(CC) -c $(CFLAGS) test_r.c -o test_r.o

test_indiv.o: test_indiv.c
	$(CC) -c $(CFLAGS) test_indiv.c -o test_indiv.o

tests.o: tests.c
	$(CC) -c $(CFLAGS) tests.c -o tests.o

rcc:
	cd .. && make

clean:
	rm -f *.o *~ a.out *.s *.bin $(EXE) 

format:
	find . -name "*.c" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
	find . -name "*.h" | xargs indent -par -br -brf -brs -kr -ci2 -cli2 -i2 -l80 -nut
