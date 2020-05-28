all: mypipeline.bin

mypipeline.bin: mypipeline.o
	gcc -g -Wall -o mypipeline.bin mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -Wall -c -o mypipeline.o mypipeline.c

.PHONY: clean

clean: 
	rm -f *.o mypipeline.bin