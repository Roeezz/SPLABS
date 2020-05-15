all: myPipe.exe

myPipe.exe: myPipe.o
	gcc -g -Wall -o myPipe.exe myPipe.o

myPipe.o: myPipe.c
	gcc -g -Wall -c -o myPipe.o myPipe.c

.PHONY: clean

clean: 
	rm -f *.o myPipe.exe