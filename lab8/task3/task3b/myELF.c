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
#define printDebug(state, ...)                                                     \
    if (state->debug_mode)                                                         \
    {                                                                              \
        fprintf(stderr, "\nDEBUG PRINT:\n");                                       \
        fprintf(stderr, "####################################################\n"); \
        fprintf(stderr, __VA_ARGS__);                                              \
        fprintf(stderr, "####################################################\n"); \
    }

#define newline(stream) fprintf(stream, "\n");

typedef struct state
{
    char debug_mode;
    int page_size;

    char file_name[BUF_SIZE];
    FILE *elf_file;
    void *map_ptr;
    size_t map_size;
    Elf32_Ehdr *header;

    int ext_stat;
} state;

typedef struct fun_desc
{
    char *desc;
    void (*func)(state *);
} FunDesc;

typedef struct PrintForm
{
    const char *format;
    long data;
} PrintForm;

bool menuRangeCheck(int index, int menu_size);
void printMenu(FunDesc menu[]);
void getInput(char *prompt, char *format, int length, char input[length]);
FILE *tryfopen(char *filename, char *mode, char *errmsg);

void toggelDebug(state *pstate);
void examinElf(state *pstate);
void printSectionNames(state *pstate);
void printSymbols(state *pstate);
void printReltabs(state *pstate);
void quit(state *pstate);

bool isElfFile(void *map_ptr);
char *getEncoding(Elf32_Ehdr *header);
char *getShTypeName(Elf32_Word sh_type);
Elf32_Shdr *getFirstShByType(Elf32_Ehdr *header, void *file_start, Elf32_Word type);
Elf32_Shdr *getFirstShByTypeOff(Elf32_Ehdr *header, void *file_start, Elf32_Word type, int *offset);
Elf32_Shdr *getShLink(Elf32_Ehdr *header, void *file_start, Elf32_Word sh_link);
Elf32_Shdr *getShdrs(Elf32_Ehdr *header, void *file_start);
int getLongestShName(Elf32_Ehdr *header, void *file_start, Elf32_Half shnum);
Elf32_Shdr *getSh(Elf32_Ehdr *header, void *file_start, Elf32_Half ndx);
char *getSpecNdxName(Elf32_Section shndx);
void *getSection(void *file_start, Elf32_Off sh_offset);
int getSecNEnt(Elf32_Shdr *sh);
char *getSectionName(Elf32_Ehdr *header, void *file_start, Elf32_Shdr *sh);
Elf32_Sym *getSymbol(Elf32_Ehdr *header, void *file_start, Elf32_Half ndx, Elf32_Word tabType);
char *getSymbolName(Elf32_Ehdr *header, void *file_start, Elf32_Sym *symbol, Elf32_Word tabType);
char *getRelTypeName(unsigned char type);

int main(int argc, char **argv)
{
    int funcIndex = 0, menuSize = 0;
    char input[BUF_SIZE];
    state *pstate = (calloc(1, sizeof(state)));
    pstate->page_size = sysconf(_SC_PAGE_SIZE);
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
        printDebug(pstate,
                   "-page_size: %d\n-file_name: %s\n-map_size: %zd\n",
                   pstate->page_size,
                   pstate->file_name,
                   pstate->map_size);
        printMenu(menu);

        getInput("Please choose an action by number: ", "%s", BUF_SIZE, input);
        funcIndex = atoi(input);
        newline(stdin);
        if (menuRangeCheck(funcIndex, menuSize))
        {
            (*menu[funcIndex].func)(pstate);
        }
        else
        {
            printf("Not within Bounds\n");
            pstate->ext_stat = 1;
            (*quit)(pstate);
        }
    }
    (*quit)(pstate);
    return 0;
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

void toggelDebug(state *pstate)
{
    pstate->debug_mode = ~pstate->debug_mode;
    pstate->debug_mode ? puts("Debug flag now on, printing debug messages") : puts("Debug flag now off, you're on your own");
}

void examinElf(state *pstate)
{
    FILE *file = NULL;
    struct stat file_stat;
    getInput("Please input file name: ", "%s", BUF_SIZE, pstate->file_name);
    if (!(file = tryfopen(pstate->file_name, "r+", "examin elf file")))
        return;
    if (fstat(file->_fileno, &file_stat) != 0)
    {
        fclose(file);
        pstate->file_name[0] = 0;
        perror("stat failed");
        return;
    }
    pstate->map_ptr = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->_fileno, 0);
    if (pstate->map_ptr < 0)
    {
        fclose(file);
        pstate->file_name[0] = 0;
        perror("examin elf file");
        return;
    }
    pstate->map_size = file_stat.st_size;
    printf("File magic number: %.4s\n", (char *)(pstate->map_ptr));
    if (!isElfFile(pstate->map_ptr))
    {
        fclose(file);
        munmap(pstate->map_ptr, pstate->map_size);
        pstate->map_size = 0;
        pstate->file_name[0] = 0;
        printf("Invalid file sh_type\n");
        return;
    }

    pstate->elf_file = file;
    pstate->header = (Elf32_Ehdr *)pstate->map_ptr;
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
           pstate->header->e_ident,
           getEncoding(pstate->header),
           pstate->header->e_entry,
           pstate->header->e_shoff,
           pstate->header->e_shnum,
           pstate->header->e_shentsize,
           pstate->header->e_phoff,
           pstate->header->e_phnum,
           pstate->header->e_phentsize);

    fclose(file);
}

void printSectionNames(state *pstate)
{
    if (pstate->map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }
    Elf32_Half shnum = pstate->header->e_shnum;
    int longest = getLongestShName(pstate->header, pstate->map_ptr, shnum);

    Elf32_Shdr *sh = NULL;
    char *secName = NULL;
    char *typeName = NULL;

    printf("\nSECTION HEADERS:\n");
    printf(" [Nr] %-*s %-8s %-6s %-6s  Type\n", longest, "Addr", "Off", "Size", "Name");
    for (int i = 0; i < shnum; i++)
    {
        sh = getSh(pstate->header, pstate->map_ptr, i);
        secName = getSectionName(pstate->header, pstate->map_ptr, sh);
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

void printSymbols(state *pstate)
{
    if (pstate->map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }

    int longest = getLongestShName(pstate->header, pstate->map_ptr, pstate->header->e_shnum);

    char *secName = NULL;
    char *symName = NULL;
    Elf32_Shdr *sh = NULL;
    Elf32_Sym *symbol = NULL;
    Elf32_Section shndx = 0;

    Elf32_Shdr *symtabSh = getFirstShByType(pstate->header, pstate->map_ptr, SHT_SYMTAB);
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
        symbol = getSymbol(pstate->header, pstate->map_ptr, i, SHT_SYMTAB);
        shndx = symbol->st_shndx;
        symName = getSymbolName(pstate->header, pstate->map_ptr, symbol, SHT_SYMTAB);
        if (shndx == SHN_ABS || shndx == SHN_UNDEF)
        {
            secName = "";
            printf("  %3d: %08x  %3s %-*s %s\n",
                   i, symbol->st_value, getSpecNdxName(shndx), longest, secName, symName);
        }
        else
        {
            sh = getSh(pstate->header, pstate->map_ptr, shndx);
            secName = getSectionName(pstate->header, pstate->map_ptr, sh);
            printf("  %3d: %08x  %3d %-*s %s\n",
                   i, symbol->st_value, shndx, longest, secName, symName);
        }
    }
}

void printReltabs(state *pstate)
{
    if (pstate->map_size == 0)
    {
        printf("No file loaded\n");
        return;
    }

    int secOff = 0;
    Elf32_Shdr *reltabSh = getFirstShByTypeOff(pstate->header, pstate->map_ptr, SHT_REL, &secOff);
    Elf32_Rel *reltab = NULL;
    char * reltabName = NULL;
    int relnum = 0;
    Elf32_Word relInfo = 0;
    Elf32_Word relOff = 0;
    unsigned char relType = 0;
    Elf32_Half symNdx = 0;
    Elf32_Sym *symbol = NULL;
    char *symName = NULL;

    while (reltabSh)
    {
        reltab = getSection(pstate->map_ptr, reltabSh->sh_offset);
        relnum = getSecNEnt(reltabSh);
        reltabName = getSectionName(pstate->header, pstate->map_ptr, reltabSh);
        printf("\nRelocation section '%s' at offset %#x contains %d entries:\n", reltabName, reltabSh->sh_offset, relnum);
        printf(" Offset     Info    Type            Sym.Value  Sym. Name\n");
        for (int i = 0; i < relnum; i++)
        {
            relInfo = reltab[i].r_info;
            relOff = reltab[i].r_offset;
            relType = ELF32_R_TYPE(relInfo);
            symNdx = ELF32_R_SYM(relInfo);
            symbol = getSymbol(pstate->header, pstate->map_ptr, symNdx, SHT_DYNSYM);
            symName = getSymbolName(pstate->header, pstate->map_ptr, symbol, SHT_DYNSYM);
            printf("%08x  %08x %-14s   %08x   %s\n",
            relOff, relInfo, getRelTypeName(relType), symbol->st_value, symName);
        }
        reltabSh = getFirstShByTypeOff(pstate->header, pstate->map_ptr, SHT_REL, &secOff);
    }
}

void quit(state *pstate)
{
    int exitStatus = pstate->ext_stat;
    if (pstate->map_size > 0)
        munmap(pstate->map_ptr, pstate->map_size);
    if (pstate->elf_file)
        fclose(pstate->elf_file);
    free(pstate);
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

/*Returns a string representing the encoding of the ELF file with `header`*/
char *getEncoding(Elf32_Ehdr *header)
{
    char *encodings[3] = {"Invalid encoding",
                          "2's complement, little endian",
                          "2's complement, big endian"};
    return encodings[header->e_ident[5]];
}

/*Checks if geven mapping is of an ELF file*/
bool isElfFile(void *map_ptr)
{
    if (memcmp(map_ptr + 1, "ELF", 3) == 0)
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
Elf32_Shdr *getFirstShByType(Elf32_Ehdr *header, void *file_start, Elf32_Word type)
{
    int pass = 0;
    return getFirstShByTypeOff(header, file_start, type, &pass);
}

/*Starting at number in `offset`, returns the first section header with sh_type `type` if it exists, NULL otherwise.
If this function finds a suitable section header, it sets the number in `offset` to the next index after the found sectoin header, 
othewise it sets it to -1*/
Elf32_Shdr *getFirstShByTypeOff(Elf32_Ehdr *header, void *file_start, Elf32_Word type, int *offset)
{
    Elf32_Half shnum = header->e_shnum;
    Elf32_Shdr *sh_arr = getShdrs(header, file_start);
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
Elf32_Shdr *getShLink(Elf32_Ehdr *header, void *file_start, Elf32_Word sh_link)
{
    Elf32_Shdr *sh_arr = getShdrs(header, file_start);
    return &sh_arr[sh_link];
}

/*Returns the section headers of file with `header`*/
Elf32_Shdr *getShdrs(Elf32_Ehdr *header, void *file_start)
{
    Elf32_Off shoff = header->e_shoff;
    return file_start + shoff;
}

/*Calculates and returns the length of the longest section name in `sh_arr`*/
int getLongestShName(Elf32_Ehdr *header, void *file_start, Elf32_Half shnum)
{
    Elf32_Shdr *sh_strtabSh = getSh(header, file_start, header->e_shstrndx);
    char *sh_strtab = getSection(file_start, sh_strtabSh->sh_offset);
    Elf32_Shdr *sh_arr = getShdrs(header, file_start);
    int longest = 0, current = 0;
    Elf32_Shdr *sh;
    char *secName;
    for (int i = 0; i < shnum; i++)
    {
        sh = &sh_arr[i];
        secName = &sh_strtab[sh->sh_name];
        current = strlen(secName);
        if (current > longest)
            longest = current;
    }
    return longest;
}

/*Returns the section header with index `ndx` in the file with `header`*/
Elf32_Shdr *getSh(Elf32_Ehdr *header, void *file_start, Elf32_Half ndx)
{
    return &getShdrs(header, file_start)[ndx];
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
void *getSection(void *file_start, Elf32_Off sh_offset)
{
    return file_start + sh_offset;
}
/*Returns amount of entrys in section with header `sh`*/
int getSecNEnt(Elf32_Shdr *sh)
{
    return sh->sh_size / sh->sh_entsize;
}

/*Return the section name with header `sh`*/
char *getSectionName(Elf32_Ehdr *header, void *file_start, Elf32_Shdr *sh)
{
    Elf32_Shdr *sh_strtabSh = getSh(header, file_start, header->e_shstrndx);
    char *sh_strtab = getSection(file_start, sh_strtabSh->sh_offset);
    return &sh_strtab[sh->sh_name];
}

Elf32_Sym *getSymbol(Elf32_Ehdr *header, void *file_start, Elf32_Half ndx, Elf32_Word tabType)
{
    Elf32_Shdr *symtabSh = getFirstShByType(header, file_start, tabType);
    if (!symtabSh)
    {
        printf("Invalid ELF: no symbol table found");
        return NULL;
    }
    Elf32_Sym *symtab = getSection(file_start, symtabSh->sh_offset);
    return &symtab[ndx];
}

/*Returns the name of `symbol`. If there is no symbol table returns NULL*/
char *getSymbolName(Elf32_Ehdr *header, void *file_start, Elf32_Sym *symbol, Elf32_Word tabType)
{
    Elf32_Shdr *symtabSh = getFirstShByType(header, file_start, tabType);
    if (!symtabSh)
    {
        printf("Invalid ELF: no symbol table found");
        return NULL;
    }
    Elf32_Shdr *sym_strtabSh = getShLink(header, file_start, symtabSh->sh_link);
    char *sym_strtab = getSection(file_start, sym_strtabSh->sh_offset);
    return &sym_strtab[symbol->st_name];
}

char *getRelTypeName(unsigned char type)
{
    char *relTypes[] = {"R_386_NONE", "R_386_32", "R_386_PC32", "R_386_GOT32",
                        "R_386_PLT32", "R_386_COPY", "R_386_GLOB_DAT", "R_386_JMP_SLOT",
                        "R_386_RELATIVE", "R_386_GOTOFF", "R_386_GOTPC"};
    return relTypes[type];
}