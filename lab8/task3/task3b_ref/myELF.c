#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LOW_CHAR 0x20
#define HIGH_CHAR 0x7E
#define NULL_CHAR '\0'
#define INITIAL_SIZE 5
#define BUF_SIZE 128
#define printDebug(...)                                                            \
    if (debug_mode)                                                                \
    {                                                                              \
        fprintf(stderr, "\nDEBUG PRINT:\n");                                       \
        fprintf(stderr, "####################################################\n"); \
        fprintf(stderr, __VA_ARGS__);                                              \
        fprintf(stderr, "####################################################\n"); \
    }

#define newline(stream) fprintf(stream, "\n");

char debug_mode;
int page_size;

char _file_name[BUF_SIZE];
FILE *_elf_file;
void *_map_ptr;
size_t _map_size;
Elf32_Ehdr *_header;

int _ext_stat;

typedef struct fun_desc
{
    char *desc;
    void (*func)();
} FunDesc;

void initGlobals();
bool menuRangeCheck(int index, int menu_size);
void printMenu(FunDesc menu[]);
void getInput(char *prompt, char *format, int length, char input[length]);
FILE *tryfopen(char *filename, char *mode, char *errmsg);

void toggelDebug();
void examinElf();
void printSectionNames();
void printSymbols();
void printReltabs();
void quit();

bool isElfFile(void *_map_ptr);
char *getEncoding();
char *getShTypeName(Elf32_Word sh_type);
Elf32_Shdr *getFirstShByType(Elf32_Word type);
Elf32_Shdr *getFirstShByTypeOff(Elf32_Word type, int *offset);
Elf32_Shdr *getShLink(Elf32_Word sh_link);
Elf32_Shdr *getShdrs();
int getLongestShName();
Elf32_Shdr *getSh(Elf32_Half ndx);
char *getSpecNdxName(Elf32_Section shndx);
void *getSection(Elf32_Off sh_offset);
int getSecNEnt(Elf32_Shdr *sh);
char *getSectionName(Elf32_Shdr *sh);
Elf32_Sym *getSymbol(Elf32_Half ndx, Elf32_Word tabType);
char *getSymbolName(Elf32_Sym *symbol, Elf32_Word tabType);
char *getRelTypeName(unsigned char type);

int main(int argc, char **argv)
{
    int funcIndex = 0, menuSize = 0;
    char input[BUF_SIZE];
    initGlobals();
    FunDesc menu[] = {{"Toggle Degbug Mode", toggelDebug},
                      {"Examin ELF File", examinElf},
                      {"Print Section Names", printSectionNames},
                      {"Print Symbols", printSymbols},
                      {"Relocation Tables", printReltabs},
                      {"Quit", quit},
                      {NULL, NULL}};

    for (int i = 0; menu[i].func != NULL; i++)
    {
        menuSize++;
    }
    while (true)
    {
        printDebug("-_file_name: %s\n-_map_size: %zd\n",
                   _file_name,
                   _map_size);
        printMenu(menu);

        getInput("Please choose an action by number: ", "%s", BUF_SIZE, input);
        funcIndex = atoi(input);
        newline(stdin);
        if (menuRangeCheck(funcIndex, menuSize))
        {
            (*menu[funcIndex].func)();
        }
        else
        {
            printf("Not within Bounds\n");
            _ext_stat = 1;
            (*quit)();
        }
    }
    (*quit)();
    return 0;
}

void initGlobals()
{
    _file_name[0] = 0;
    _elf_file = NULL;
    _map_ptr = NULL;
    _map_size = 0;
    _header = NULL;

    _ext_stat = 0;
}

bool menuRangeCheck(int index, int menu_size)
{
    return index >= 0 && index < menu_size;
}

void printMenu(FunDesc menu[])
{
    printf("\nACTIONS MENU:\n");
    printf("----------------------------------------------------\n");
    for (int i = 0; menu[i].desc != NULL; i++)
    {
        printf("%i-%s\n", i, menu[i].desc);
    }
    printf("----------------------------------------------------\n");
}

void toggelDebug()
{
    debug_mode = ~debug_mode;
    debug_mode ? puts("Debug flag now on, printing debug messages") : puts("Debug flag now off, you're on your own");
}

void examinElf()
{
    FILE *file = NULL;
    struct stat file_stat;
    getInput("Please input file name: ", "%s", BUF_SIZE, _file_name);
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
    printf("\nELF HEADER:\n"
           "  MAGIC:                      %.4s\n"
           "  Data:                       %s\n"
           "  Entry point address:        %x\n"
           "  Start of section headers:   %u\n"
           "  Number of section headers:  %hu\n"
           "  Size of section headers:    %hu (bytes)\n"
           "  Start of program headers:   %u\n"
           "  Number of program headers:  %hu\n"
           "  Size of program headers:    %hu (bytes)\n",
           _header->e_ident,
           getEncoding(_header),
           _header->e_entry,
           _header->e_shoff,
           _header->e_shnum,
           _header->e_shentsize,
           _header->e_phoff,
           _header->e_phnum,
           _header->e_phentsize);
}

void printSectionNames()
{
    if (_map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }
    int longest = getLongestShName();

    Elf32_Shdr *sh = NULL;
    char *secName = NULL;
    char *typeName = NULL;

    printf("\nSECTION HEADERS:\n");
    printf(" [Nr] %-*s %-8s %-6s %-6s  Type\n", longest, "Addr", "Off", "Size", "Name");
    for (int i = 0; i < _header->e_shnum; i++)
    {
        sh = getSh(i);
        secName = getSectionName(sh);
        typeName = getShTypeName(sh->sh_type);
        printf(" [%2d] %-*s %08x %06x %06x  %s\n",
               i, longest,
               secName,
               sh->sh_addr,
               sh->sh_offset,
               sh->sh_size,
               typeName);
    }
}

void printSymbols()
{
    if (_map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }

    int longest = getLongestShName();

    char *secName = NULL;
    char *symName = NULL;
    Elf32_Shdr *sh = NULL;
    Elf32_Sym *symbol = NULL;
    Elf32_Section shndx = 0;

    Elf32_Shdr *symtabSh = getFirstShByType(SHT_SYMTAB);
    if (!symtabSh)
    {
        printf("Invalid ELF: no symbol table found");
        return;
    }
    int symnum = getSecNEnt(symtabSh);
    printf("\nSYMBOL TABLE:\n");
    printf("  Num: %8s  Ndx %-*s Name\n", "Value", longest, "Section");
    for (int i = 0; i < symnum; i++)
    {
        symbol = getSymbol(i, SHT_SYMTAB);
        shndx = symbol->st_shndx;
        symName = getSymbolName(symbol, SHT_SYMTAB);
        if (shndx == SHN_ABS || shndx == SHN_UNDEF)
        {
            secName = "";
            printf("  %3d: %08x  %3s %-*s %s\n",
                   i, symbol->st_value, getSpecNdxName(shndx), longest, secName, symName);
        }
        else
        {
            sh = getSh(shndx);
            secName = getSectionName(sh);
            printf("  %3d: %08x  %3d %-*s %s\n",
                   i, symbol->st_value, shndx, longest, secName, symName);
        }
    }
}

void printReltabs()
{
    if (_map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }

    int secOff = 0;
    Elf32_Shdr *reltabSh = getFirstShByTypeOff(SHT_REL, &secOff);
    Elf32_Rel *reltab = NULL;
    char *reltabName = NULL;
    int relnum = 0;
    Elf32_Word relInfo = 0;
    Elf32_Word relOff = 0;
    unsigned char relType = 0;
    Elf32_Half symNdx = 0;
    Elf32_Sym *symbol = NULL;
    char *symName = NULL;

    while (reltabSh)
    {
        reltab = getSection(reltabSh->sh_offset);
        relnum = getSecNEnt(reltabSh);
        reltabName = getSectionName(reltabSh);
        printf("\nRelocation section '%s' at offset %#x contains %d entries:\n", reltabName, reltabSh->sh_offset, relnum);
        printf(" Offset     Info    Type            Sym.Value  Sym. Name\n");
        for (int i = 0; i < relnum; i++)
        {
            relInfo = reltab[i].r_info;
            relOff = reltab[i].r_offset;
            relType = ELF32_R_TYPE(relInfo);
            symNdx = ELF32_R_SYM(relInfo);
            symbol = getSymbol(symNdx, SHT_DYNSYM);
            symName = getSymbolName(symbol, SHT_DYNSYM);
            printf("%08x  %08x %-14s   %08x   %s\n",
                   relOff, relInfo, getRelTypeName(relType), symbol->st_value, symName);
        }
        reltabSh = getFirstShByTypeOff(SHT_REL, &secOff);
    }
}

void quit()
{
    int exitStatus =_ext_stat;
    if (_map_size > 0)
        munmap(_map_ptr, _map_size);
    if (_elf_file)
        fclose(_elf_file);
    printf("Exiting. status: %d\n", exitStatus);
    exit(exitStatus);
}

/*Prompts input from user usnig `prompt`, then SAFELY reads `length` input into `input`, using `format`*/
void getInput(char *prompt, char *format, int length, char input[length])
{
    printf("%s", prompt);
    char tmpBuf[length];
    fgets(tmpBuf, length, stdin);
    sscanf(tmpBuf, format, input);
}

/*Tries to open `filename` with fopen, using `mode`. On success returns the FILE* returnd by fopen.
On fail prints `errmsg` using perror and returns NULL.*/
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

/*Returns a string representing the encoding of the ELF file with `_header`*/
char *getEncoding()
{
    char *encodings[3] = {"Invalid encoding",
                          "2's complement, little endian",
                          "2's complement, big endian"};
    return encodings[_header->e_ident[5]];
}

/*Checks if geven mapping is of an ELF file*/
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

/*Get's the type name corresponding with `type`*/
char *getShTypeName(Elf32_Word sh_type)
{
    if (sh_type <= 11)
    {
        char *types[12] = {"NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH",
                           "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM"};
        return types[sh_type];
    }
    switch (sh_type)
    {
    case SHT_LOPROC:
        return "LOPROC";
    case SHT_HIPROC:
        return "HIPROC";
    case SHT_LOUSER:
        return "LOUSER";
    case SHT_HIUSER:
        return "HIUSER";
    default:
        return "UNKNOWN";
    }
}

/*Returns the first section with sh_type `type`, if it exists, NULL otherwise*/
Elf32_Shdr *getFirstShByType(Elf32_Word type)
{
    int pass = 0;
    return getFirstShByTypeOff(type, &pass);
}

/*Starting at number in `offset`, returns the first section _header with sh_type `type` if it exists, NULL otherwise.
If this function finds a suitable section _header, it sets the number in `offset` to the next index after the found sectoin _header, 
othewise it sets it to -1*/
Elf32_Shdr *getFirstShByTypeOff(Elf32_Word type, int *offset)
{
    Elf32_Half shnum = _header->e_shnum;
    Elf32_Shdr *sh_arr = getShdrs(_header, _map_ptr);
    Elf32_Shdr *sh;
    for (int i = *offset; i < shnum; i++)
    {
        sh = &sh_arr[i];
        if (sh->sh_type == type)
        {
            *offset = i + 1;
            return sh;
        }
    }
    *offset = -1;
    return NULL;
}

/*Returns given link from the file sections headers*/
Elf32_Shdr *getShLink(Elf32_Word sh_link)
{
    Elf32_Shdr *sh_arr = getShdrs();
    return &sh_arr[sh_link];
}

/*Returns the section headers of file with `_header`*/
Elf32_Shdr *getShdrs()
{
    Elf32_Off shoff = _header->e_shoff;
    return _map_ptr + shoff;
}

/*Calculates and returns the length of the longest section name in `sh_arr`*/
int getLongestShName()
{
    Elf32_Shdr *sh_strtabSh = getSh(_header->e_shstrndx);
    char *sh_strtab = getSection(sh_strtabSh->sh_offset);
    Elf32_Shdr *sh_arr = getShdrs();
    int longest = 0, current = 0;
    Elf32_Shdr *sh;
    char *secName;
    for (int i = 0; i < _header->e_shnum; i++)
    {
        sh = &sh_arr[i];
        secName = &sh_strtab[sh->sh_name];
        current = strlen(secName);
        if (current > longest)
            longest = current;
    }
    return longest;
}

/*Returns the section _header with index `ndx` in the file with `_header`*/
Elf32_Shdr *getSh(Elf32_Half ndx)
{
    return &getShdrs()[ndx];
}

/*Returns the name of special index `shndx`*/
char *getSpecNdxName(Elf32_Section shndx)
{
    switch (shndx)
    {
    case SHN_ABS:
        return "ABS";
    case SHN_UNDEF:
        return "UND";
    default:
        return "UNK";
    }
}

/*Returns the section at `sh_offset` in file*/
void *getSection(Elf32_Off sh_offset)
{
    return _map_ptr + sh_offset;
}
/*Returns amount of entrys in section with _header `sh`*/
int getSecNEnt(Elf32_Shdr *sh)
{
    return sh->sh_size / sh->sh_entsize;
}

/*Return the section name with _header `sh`*/
char *getSectionName(Elf32_Shdr *sh)
{
    Elf32_Shdr *sh_strtabSh = getSh(_header->e_shstrndx);
    char *sh_strtab = getSection(sh_strtabSh->sh_offset);
    return &sh_strtab[sh->sh_name];
}

/*Returns the symbol with index `ndx` from symbol table with type `tabType`*/
Elf32_Sym *getSymbol(Elf32_Half ndx, Elf32_Word tabType)
{
    Elf32_Shdr *symtabSh = getFirstShByType(tabType);
    if (!symtabSh)
    {
        printf("Invalid ELF: no symbol table found");
        return NULL;
    }
    Elf32_Sym *symtab = getSection(symtabSh->sh_offset);
    return &symtab[ndx];
}

/*Returns the name of `symbol`. If there is no symbol table returns NULL*/
char *getSymbolName(Elf32_Sym *symbol, Elf32_Word tabType)
{
    Elf32_Shdr *symtabSh = getFirstShByType(tabType);
    if (!symtabSh)
    {
        printf("Invalid ELF: no symbol table found");
        return NULL;
    }
    Elf32_Shdr *sym_strtabSh = getShLink(symtabSh->sh_link);
    char *sym_strtab = getSection(sym_strtabSh->sh_offset);
    return &sym_strtab[symbol->st_name];
}

/*Returns the type name of the relocation type*/
char *getRelTypeName(unsigned char type)
{
    char *relTypes[] = {"R_386_NONE", "R_386_32", "R_386_PC32", "R_386_GOT32",
                        "R_386_PLT32", "R_386_COPY", "R_386_GLOB_DAT", "R_386_JMP_SLOT",
                        "R_386_RELATIVE", "R_386_GOTOFF", "R_386_GOTPC"};
    return relTypes[type];
}