#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SIG_SIZ_LEN 2
#define VIR_NAM_LEN 16
#define BUF_SIZE 50
#define IND_OFFSET 1

typedef struct VIRUS
{
	unsigned short sigSize;
	char virusName[VIR_NAM_LEN];
	char *sig;
} VIRUS;

typedef struct Link Link;

struct Link
{
	Link *nextVirus;
	VIRUS *vir;
};

typedef struct fun_desc
{
	char *name;
	Link *(*func)(Link *);
} FunDesc;

//VIRUS rule of 5
void VIRUSctor(VIRUS *virus, unsigned short sigSize, char virusName[], char *sig);
void VIRUSdtor(VIRUS *virus);
VIRUS *VIRUScreate(unsigned short sigSize, char virusName[], char *sig);
void VIRUSdestroy(VIRUS *virus);

//antivirus functions
VIRUS *readVirus(FILE *virusFile);
void printVirus(VIRUS *virus, FILE *output);
void printSignature(VIRUS *virus, FILE *output);
void printHex(char *buffer, int length, FILE *output);

//linked list functions
void list_print(Link *virus_list, FILE *);
Link *list_append(Link *virus_list, VIRUS *data);
void list_free(Link *virus_list);

//menu functions
Link *loadSignatures(Link *virusList);
Link *printSignatures(Link *virusList);
Link *quit(Link *virusList);

bool menuRangeCheck(int index, int menu_size);

int main(int argc, char **argv)
{
	int menu_size = 0, funcIndex = 0;
	char input[BUF_SIZE], inputTmp[BUF_SIZE];
	Link *virusList = NULL;

	FunDesc menu[] = {{"Load signatures", &loadSignatures},
					  {"Print signatures", &printSignatures},
					  {"Quit", &quit},
					  {NULL, NULL}};

	for (int i = 0; menu[i].func != NULL; i++)
	{
		menu_size++;
	}
	while (true)
	{
		printf("Actions menu:\n");
		for (int i = 0; menu[i].func != NULL; i++)
		{
			printf("%i)   %s\n", i + 1, menu[i].name);
		}

		printf("Please choose an action by number: ");
		fgets(input, BUF_SIZE, stdin);
		sscanf(input, "%s", inputTmp);
		funcIndex = atoi(inputTmp) - IND_OFFSET;
		if (menuRangeCheck(funcIndex, menu_size))
		{
			virusList = menu[funcIndex].func(virusList);
		}
		else
		{
			printf("Not within Bounds\n");
			(*quit)(virusList);
		}
	}
	return 0;
}

bool menuRangeCheck(int index, int menu_size)
{
	return index >= 0 && index < menu_size;
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
	fprintf(output, "Virus name: %s\nVirus size: %d\nSignature:\n", virus->virusName, virus->sigSize);
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

void VIRUSdtor(VIRUS *virus)
{
	virus->sigSize = 0;
	virus->virusName[0] = 0;

	free(virus->sig);
	virus->sig = NULL;
}

void VIRUSdestroy(VIRUS *virus)
{
	VIRUSdtor(virus);
	free(virus);
}

void LINKctor(Link *link, VIRUS *data)
{
	link->nextVirus = NULL;
	link->vir = data;
}

Link *LINKcreate(VIRUS *data)
{
	Link *newLink = malloc(sizeof(Link));
	newLink->nextVirus = NULL;
	newLink->vir = data;
	return newLink;
}

void list_print(Link *virus_list, FILE *output)
{
	if (virus_list != NULL)
	{
		list_print(virus_list->nextVirus, output);
		printVirus(virus_list->vir, output);
	}
}

Link *list_append(Link *virus_list, VIRUS *data)
{	
	Link *newLink = LINKcreate(data);
	newLink->nextVirus = virus_list;
	return newLink;
}

void list_free(Link *virus_list)
{
	if (virus_list != NULL)
	{
		VIRUSdestroy(virus_list->vir);
		list_free(virus_list->nextVirus);
		free(virus_list);
		virus_list = NULL;
	}
}

Link *loadSignatures(Link *virusList)
{
	char filenameTmp[BUF_SIZE], filename[BUF_SIZE];
	FILE *virusFile = NULL;
	VIRUS *virus = NULL;
	Link *list = virusList;

	fgets(filenameTmp, BUF_SIZE, stdin);
	sscanf(filenameTmp, "%s", filename);
	virusFile = fopen(filename, "r");

	while (virusFile != NULL && fgetc(virusFile) != EOF)
	{
		fseek(virusFile, -1, SEEK_CUR);
		virus = readVirus(virusFile);
		list = list_append(list, virus);
	}
	if (virusFile == NULL)
	{
		printf("Could not open file.\n");
	}
	else {
		fclose(virusFile);
	}
	return list;
}

Link *printSignatures(Link *virusList)
{
	if (virusList != NULL)
	{
		list_print(virusList, stdout);
	}
	return virusList;
}

Link *quit(Link *virusList)
{
	list_free(virusList);
	virusList = NULL;
	exit(0);
}