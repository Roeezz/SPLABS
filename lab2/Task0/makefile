all: count-words

count-words: count-words.o
	gcc  -g -m32 -o count-words count-words.o
	rm count-words.o

count-words.o: count-words.c
	gcc  -g -m32 -c -o count-words.o count-words.c

.PHONY: clean
clean:
	rm -rf ./*.o count-words
