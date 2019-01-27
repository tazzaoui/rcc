CXX=g++
CXX_FLAGS=--std=c++11 -Wall -pedantic
TESTS=test test-interp test-2n test-randp
EXE=rcc $(TESTS) 

all: $(EXE) 

rcc: r0.o main.o
	$(CXX) main.o r0.o -o rcc

test: r0.o test.o 
	$(CXX) test.o r0.o -o test

test-interp: r0.o test-interp.o
	$(CXX) test-interp.o r0.o -o test-interp

test-2n: r0.o test-2n.o
	$(CXX) test-2n.o r0.o -o test-2n

test-randp: r0.o test-randp.o
	$(CXX) test-randp.o r0.o -o test-randp

main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) main.cpp -o main.o

test.o: tests/test.cpp
	$(CXX) -c $(CXX_FLAGS) tests/test.cpp -o test.o

test-interp.o: tests/test_interp.cpp
	$(CXX) -c $(CXX_FLAGS) tests/test_interp.cpp -o test-interp.o

test-2n.o: tests/test_2n.cpp
	$(CXX) -c $(CXX_FLAGS) tests/test_2n.cpp -o test-2n.o

test-randp.o: tests/test_randp.cpp
	$(CXX) -c $(CXX_FLAGS) tests/test_randp.cpp -o test-randp.o

r0.o: r0.cpp r0.hpp
	$(CXX) -c $(CXX_FLAGS) r0.cpp -o r0.o

clean:
	rm -f *.o *~ $(EXE) 
