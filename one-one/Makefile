test : thread.o test.o
	cc thread.o test.o -o test
thread.o : thread.c ../thread.h
	cc -c -Wall thread.c
test.o: test.c ../thread.h
	cc -c -Wall test.c
