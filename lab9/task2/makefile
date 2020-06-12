all: virus

virus: skeleton.o
	ld -m elf_i386 skeleton.o -o virus.bin

skeleton.o: skeleton.s
	nasm -f elf32 skeleton.s -o skeleton.o
.PHONY: clean
clean: 
	rm -f *.o virus.bin
