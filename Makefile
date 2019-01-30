CXX=g++
CXX_FLAGS=--std=c++11 -Wall -pedantic
TESTS=test
EXE=rcc $(TESTS) 

all: $(EXE) 

rcc: rcc.o main.o
	$(CXX) main.o rcc.o -o rcc

test: rcc.o main_tests.o tests.o 
	$(CXX) main_tests.o tests.o rcc.o -o test

main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) main.cpp -o main.o

main_tests.o: tests/main.cpp
	$(CXX) -c $(CXX_FLAGS) tests/main.cpp -o main_tests.o

tests.o: tests/tests.cpp
	$(CXX) -c $(CXX_FLAGS) tests/tests.cpp -o tests.o

rcc.o: rcc.cpp rcc.hpp
	$(CXX) -c $(CXX_FLAGS) rcc.cpp -o rcc.o

clean:
	rm -f *.o *~ $(EXE) 

format:
	clang-format -i -style=google ./*.*pp tests/*.*pp
