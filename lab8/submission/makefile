all: task1 task2 task3

task1: task1.o
	gcc -g -m32 -Wall -o task1 task1.o

task2: task2.o
	gcc -g -m32 -Wall -o task2 task2.o

task3: task3.o
	gcc -g -m32 -Wall -o task3 task3.o

task1.o: task1.c
	gcc -g -m32 -Wall -c -o task1.o task1.c

task2.o: task2.c
	gcc -g -m32 -Wall -c -o task2.o task2.c

task3.o: task3.c
	gcc -g -m32 -Wall -c -o task3.o task3.c

.PHONY: clean

clean:
	rm -f *.o task1 task2 task3
