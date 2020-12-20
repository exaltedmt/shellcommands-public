all:
	g++ -c -g -Wall -std=c++14 -O0 -pedantic-errors 1730sh.cpp 
	g++ -c -g -Wall -std=c++14 -O0 -pedantic-errors Shell.cpp
	g++ -c -g -Wall -std=c++14 -O0 -pedantic-errors builtin.cpp 
	g++ -g -Wall -std=c++14 -O0 -pedantic-errors -o 1730sh 1730sh.o Shell.o builtin.o
clean:
	rm -rf 1730sh
	rm -rf *.cpp~
	rm -rf *.o
	rm -rf *.h~
	rm -rf Makefile~
	rm -rf core.*