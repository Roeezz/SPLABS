CC		:=	gcc -m32
CC_Flags	:=	-Wall -g

all: task1c

task1c: task1c.o
	$(CC) -o task1c task1c.o

%.o: %.c
	$(CC) $(CC_Flags) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *.o task1c