all: looper.bin

looper.bin: looper.o
	gcc -g -Wall -o looper.bin looper.o

looper.o: looper.c
	gcc -g -Wall -c -o looper.o looper.c

.PHONY: clean

clean: 
	rm -f *.o looper.bin