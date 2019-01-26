CXX=g++
CXX_FLAGS=--std=c++11 -Wall -pedantic

r0cc: r0.o main.o
	$(CXX) main.o r0.o -o r0cc

main.o: main.cpp
	$(CXX) -c $(CXX_FLAGS) main.cpp -o main.o

r0.o: r0.cpp r0.hpp
	$(CXX) -c $(CXX_FLAGS) r0.cpp -o r0.o

clean:
	rm -f *.o *~
