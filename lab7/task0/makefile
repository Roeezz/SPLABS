hexeditplus: hexeditplus.o
	gcc -g -Wall -o hexeditplus.bin hexeditplus.o

hexeditplus.o: hexeditplus.c
	gcc -g -Wall -c -o hexeditplus.o hexeditplus.c

.PHONY: clean

clean:
	rm -f *.o hexeditplus.bin