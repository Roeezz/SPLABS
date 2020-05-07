CC		:=	gcc -m32
CC_Flags	:=	-Wall -g

OBJ_DIR		:=	obj
BIN_DIR		:=	bin
SRC_DIR		:=	src

all: directories sort toHex
	rm -rf ${OBJ_DIR}/
	
directories:
	mkdir ${BIN_DIR}
	mkdir ${OBJ_DIR}

sort: ${OBJ_DIR}/sort.o
	$(CC) -o ${BIN_DIR}/sort ${OBJ_DIR}/sort.o

toHex: ${OBJ_DIR}/toHex.o
	$(CC) -o ${BIN_DIR}/toHex ${OBJ_DIR}/toHex.o

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c
	$(CC) $(CC_Flags) -c $< -o $@

.PHONY: clean
clean:
	rm -rf bin/ obj/