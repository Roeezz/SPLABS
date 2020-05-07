all: fucntions

fucntions: fucntions.o
	gcc  -g -m32 -o fucntions fucntions.o
	rm fucntions.o

fucntions.o: fucntions.c
	gcc  -g -m32 -c -o fucntions.o fucntions.c

.PHONY: clean
clean:
	rm -rf ./*.o fucntions
