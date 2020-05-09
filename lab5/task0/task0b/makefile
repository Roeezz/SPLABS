all: looper.exe

looper.exe: looper.o
	gcc -g -Wall -o looper.exe looper.o

looper.o: looper.c
	gcc -g -Wall -c -o looper.o looper.c

.PHONY: clean

clean: 
	rm -f *.o looper.exe