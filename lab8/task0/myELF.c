#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
    char file_name[BUF_SIZE];
    int page_size;
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
void quit(state *pstate);

int main(int argc, char **argv)
{
    int funcIndex = 0, menuSize = 0;
    char input[BUF_SIZE];
    state *pstate = (calloc(1, sizeof(state)));
    pstate->page_size = sysconf(_SC_PAGE_SIZE);
    FunDesc menu[] = {{"Toggle Degbug Mode", &toggelDebug},
                      {"Examin ELF File", &examinElf},
                      {"Quit", &quit},
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
    if (!(file = tryfopen(pstate->file_name, "r", "examin elf file")))
        return;
    if (fstat(file->_fileno, &file_stat) != 0)
    {
        perror("stat failed");
        return;
    }
    pstate->map_ptr = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->_fileno, 0);
    if (!pstate->map_ptr)
    {
        perror("examin elf file");
        return;
    }
    pstate->map_size = file_stat.st_size;
    pstate->header = (Elf32_Ehdr *)pstate->map_ptr;

    printf("-MAGIC: %.4s\n", pstate->header->e_ident);
    printf("-Entry: %x\n", pstate->header->e_entry);

    fclose(file);
}

void quit(state *pstate)
{
    int exitStatus = pstate->ext_stat;
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