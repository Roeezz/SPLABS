all: addresses

addresses: addresses.o
	gcc  -g -m32 -o addresses addresses.o
	rm addresses.o

addresses.o: addresses.c
	gcc  -g -m32 -c -o addresses.o addresses.c

.PHONY: clean
clean:
	rm -rf ./*.o addresses
