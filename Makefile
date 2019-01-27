CXX=g++
CXX_FLAGS=--std=c++11 -Wall -pedantic
TESTS=test
EXE=rcc $(TESTS) 

all: $(EXE) 

rcc: r0.o main.o
	$(CXX) main.o r0.o -o rcc

test: r0.o main_tests.o tests.o 
	$(CXX) main_tests.o tests.o r0.o -o test

main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) main.cpp -o main.o

main_tests.o: tests/main.cpp
	$(CXX) -c $(CXX_FLAGS) tests/main.cpp -o main_tests.o

tests.o: tests/tests.cpp
	$(CXX) -c $(CXX_FLAGS) tests/tests.cpp -o tests.o

r0.o: r0.cpp r0.hpp
	$(CXX) -c $(CXX_FLAGS) r0.cpp -o r0.o

clean:
	rm -f *.o *~ $(EXE) 
