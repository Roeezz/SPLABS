#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 128

void examinElf();
void printProgramHeadres();
void quit(int exitStatus);

FILE *tryfopen(char *filename, char *mode, char *errmsg);
bool isElfFile(void *_map_ptr);
Elf32_Phdr *getPh(Elf32_Half ndx);
Elf32_Phdr *getPhdrs();

char _file_name[BUF_SIZE];
FILE *_elf_file;
void *_map_ptr;
size_t _map_size;
Elf32_Ehdr *_header;

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Invalid amount of arguments");
        quit(1);
    }
    sscanf(argv[1], "%s", _file_name);
    examinElf();
    printProgramHeadres();
    quit(0);
}

void initGlobals()
{
    _file_name[0] = 0;
    _elf_file = NULL;
    _map_ptr = NULL;
    _map_size = 0;
    _header = NULL;
}

void examinElf()
{
    FILE *file = NULL;
    struct stat file_stat;
    if (!(file = tryfopen(_file_name, "r+", "examin elf file")))
        return;
    if (fstat(file->_fileno, &file_stat) != 0)
    {
        fclose(file);
        _file_name[0] = 0;
        perror("stat failed");
        return;
    }
    _map_ptr = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->_fileno, 0);
    if (_map_ptr < 0)
    {
        fclose(file);
        _file_name[0] = 0;
        perror("examin elf file");
        return;
    }
    _map_size = file_stat.st_size;
    printf("File magic number: %.4s\n", (char *)(_map_ptr));
    if (!isElfFile(_map_ptr))
    {
        fclose(file);
        munmap(_map_ptr, _map_size);
        _map_size = 0;
        _file_name[0] = 0;
        printf("Invalid file sh_type\n");
        return;
    }

    _elf_file = file;
    _header = (Elf32_Ehdr *)_map_ptr;
}

void printProgramHeadres()
{
    if (_map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }

    Elf32_Phdr *ph = NULL;

    printf("\nSECTION HEADERS:\n");
    printf(" Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n");
    for (int i = 0; i < _header->e_phnum; i++)
    {
        ph = getPh(i);
        printf(" %-14u %#08x %#010x %#010x %#07x %#07x %03x %#x\n", 
                ph->p_type, ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz, ph->p_flags, ph->p_align);
    }
}

void quit(int exitStatus)
{
    if (_map_size > 0)
        munmap(_map_ptr, _map_size);
    if (_elf_file)
        fclose(_elf_file);
    printf("Exiting. status: %d\n", exitStatus);
    exit(exitStatus);
}

bool isElfFile(void *_map_ptr)
{
    if (memcmp(_map_ptr + 1, "ELF", 3) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

Elf32_Phdr *getPh(Elf32_Half ndx)
{
    return &getPhdrs()[ndx];
}

Elf32_Phdr *getPhdrs()
{
    Elf32_Off phoff = _header->e_phoff;
    return _map_ptr + phoff;
}

FILE *tryfopen(char *filename, char *mode, char *errmsg)
{
    FILE *file;
    if (!(file = fopen(filename, mode)))
    {
        perror(errmsg);
        return NULL;
    };
    return file;
}
