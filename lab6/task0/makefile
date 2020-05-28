all: myPipe.bin

myPipe.bin: myPipe.o
	gcc -g -Wall -o myPipe.bin myPipe.o

myPipe.o: myPipe.c
	gcc -g -Wall -c -o myPipe.o myPipe.c

.PHONY: clean

clean: 
	rm -f *.o myPipe.bin