all: mypipeline.exe

mypipeline.exe: mypipeline.o
	gcc -g -Wall -o mypipeline.exe mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -Wall -c -o mypipeline.o mypipeline.c

.PHONY: clean

clean: 
	rm -f *.o mypipeline.exe