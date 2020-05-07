#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIG_SIZ_LEN 2
#define VIR_NAM_LEN 16

typedef struct VIRUS
{
    unsigned short sigSize;
    char virusName[VIR_NAM_LEN];
    char *sig;
} VIRUS;

void VIRUSctor(VIRUS *virus, unsigned short sigSize, char virusName[], char *sig);
void VIRUSdtor(VIRUS *virus);
VIRUS *VIRUScreate(unsigned short sigSize, char virusName[], char *sig);
void VIRUSdestroy(VIRUS *virus);

VIRUS *readVirus(FILE *virusFile);
void printVirus(VIRUS *virus, FILE *output);
void printSignature(VIRUS *virus, FILE *output);
void printHex(char *buffer, int length, FILE *output);

int main(int argc, char **argv)
{
    FILE *virusFile = NULL;
    FILE *output = stdout;
    VIRUS *virus = NULL;
    for (int i = 1; i < argc; i++)
    {
        if(strncmp(argv[i], "-o", 2) == 0){
            output = fopen(argv[i] + 2, "w");
        }
        if (strncmp(argv[i], "-i", 2) == 0)
        {
            virusFile = fopen(argv[++i], "r");
            if (virusFile == NULL)
            {
                printf("Could not open file. exiting, Goodbye.\n");
                exit(1);
            }
        }
    }
    while (virusFile != NULL && fgetc(virusFile) != EOF)
    {
        fseek(virusFile, -1, SEEK_CUR);
        virus = readVirus(virusFile);
        fclose(virusFile);
        printVirus(virus, output);
        VIRUSdestroy(virus);
    }
    if(virusFile == NULL){
        printf("No virus file input was enterd. exiting, Goodbye.\n");
    }
    fclose(output);
    return 0;
}

VIRUS *readVirus(FILE *virusFile)
{
    VIRUS * virus = malloc(sizeof(VIRUS));

    fread(virus, sizeof(char), SIG_SIZ_LEN + VIR_NAM_LEN, virusFile);
    
    virus->sig = malloc(virus->sigSize * sizeof(char));
    fread(virus->sig, sizeof(char), virus->sigSize, virusFile);

    return virus;
}

void printVirus(VIRUS *virus, FILE *output)
{
    fprintf(output, "VIRUS name: %s\nVirus size: %d\nSignature:\n", virus->virusName, virus->sigSize);
    printHex(virus->sig, virus->sigSize, output);
    fprintf(output, "\n");
}

void printHex(char *buffer, int length, FILE *output)
{
    for (int i = 0; i < length; i++)
    {
        fprintf(output, "%02X ", buffer[i] & 0xFF);
    }
    fprintf(output, "\n");
}


void VIRUSctor(VIRUS *virus, unsigned short sigSize, char virusName[], char *sig)
{
    virus->sigSize = sigSize;
    virus->sig = sig;

    for (int i = 0; i < 16; i++)
    {
        virus->virusName[i] = virusName[i];
    }
}

VIRUS *VIRUScreate(unsigned short sigSize, char virusName[], char *sig)
{
    VIRUS *virus = NULL;

    virus = malloc(sizeof(VIRUS));
    VIRUSctor(virus, sigSize, virusName, sig);
    return virus;
}

void VIRUSdtor(VIRUS *curVirus)
{
	free(curVirus->sig);
	curVirus->sig = NULL;
}

void VIRUSdestroy(VIRUS *virus)
{
    VIRUSdtor(virus);
    free(virus);
}