CC		:=	gcc -m32
CC_Flags	:=	-Wall -g

all: task2b

task2b: task2b.o
	$(CC) -o task2b task2b.o

%.o: %.c
	$(CC) $(CC_Flags) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *.o task2b