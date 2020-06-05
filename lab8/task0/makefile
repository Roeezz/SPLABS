myELF: myELF.o
	gcc -g -m32 -Wall -o myELF.bin myELF.o

myELF.o: myELF.c
	gcc -g -m32 -Wall -c -o myELF.o myELF.c

.PHONY: clean

clean:
	rm -f *.o myELF.bin
