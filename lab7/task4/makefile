digit_count: digit_count.o
	gcc -g -m32 -Wall -o digit_count.bin digit_count.o

digit_count.o: digit_count.c
	gcc -g -m32 -fno-pie -Wall -c -o digit_count.o digit_count.c

.PHONY: clean

clean:
	rm -f *.o digit_count.bin
