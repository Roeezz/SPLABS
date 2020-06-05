all: task1 task4

task1: task1.o
	gcc -g -m32 -Wall -o task1.bin task1.o

task1.o: task1.c
	gcc -g -m32 -Wall -c -o task1.o task1.c

task4: task4.o
	gcc -g -m32 -Wall -o task4.bin task4.o

task4.o: task4.c
	gcc -g -m32 -Wall -fno-pie -c -o task4.o task4.c

.PHONY: clean

clean:
	rm -f *.o *.bin 
