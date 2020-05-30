hexeditplus: hexeditplus.o
	gcc -g -m32 -Wall -o hexeditplus.bin hexeditplus.o

hexeditplus.o: hexeditplus.c
	gcc -g -m32 -Wall -c -o hexeditplus.o hexeditplus.c

.PHONY: clean

clean:
	rm -f *.o hexeditplus.bin