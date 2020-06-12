rpgm: rpgm.o
	gcc -g -m32 -Wall -o rpgm.bin rpgm.o

rpgm.o: rpgm.c
	gcc -g -m32 -Wall -c -o rpgm.o rpgm.c

.PHONY: clean

clean:
	rm -f *.o rpgm.bin
