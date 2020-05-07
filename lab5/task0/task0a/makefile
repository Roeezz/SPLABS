myshell: myshell.o LineParser.o
	gcc -g -Wall -o myshell myshell.o LineParser.o 

myshell.o: myshell.c
	gcc -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -c -o LineParser.o LineParser.c 

.PHONY: clean

clean: 
	rm -f *.o myshell