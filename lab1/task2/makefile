encoder: encoder.o
	gcc -g -m32 -o encoder encoder.o
	rm encoder.o

encoder.o: encoder.c
	gcc -g -m32 -c -o encoder.o encoder.c

.PHONY: clean
clean:
	rm -rf ./*.o encoder
