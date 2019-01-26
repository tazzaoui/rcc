CXX=g++
CXX_FLAGS=--std=c++11 -Wall -pedantic

all: rcc tests

rcc: r0.o main.o
	$(CXX) main.o r0.o -o rcc

tests: r0.o test.o
	$(CXX) test.o  r0.o -o tests

main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) main.cpp -o main.o

test.o: test.cpp
	$(CXX) -c $(CXX_FLAGS) test.cpp -o test.o

r0.o: r0.cpp r0.hpp
	$(CXX) -c $(CXX_FLAGS) r0.cpp -o r0.o

clean:
	rm -f *.o *~ rcc tests
