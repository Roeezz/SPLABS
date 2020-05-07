CC		:=	gcc -m32
CC_Flags	:=	-Wall -g

OBJ_DIR		:=	obj
BIN_DIR		:=	bin
SRC_DIR		:=	src

all: directories antivirus
	rm -rf ${OBJ_DIR}/

directories:
	mkdir ${BIN_DIR}
	mkdir ${OBJ_DIR}

antivirus: ${OBJ_DIR}/antivirus.o
	$(CC) -o ${BIN_DIR}/antivirus ${OBJ_DIR}/antivirus.o

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c
	$(CC) $(CC_Flags) -c $< -o $@

.PHONY: clean
clean:
	rm -rf bin/ obj/