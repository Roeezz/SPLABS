#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define SIG_SIZ_LEN 2
#define VIR_NAM_LEN 16
#define BUF_SIZE 50
#define FILE_BUF_SIZE sizeof(char) * 10 * 1000
#define IND_OFFSET 1

typedef struct VIRUS
{
	unsigned short sigSize;
	char virusName[VIR_NAM_LEN];
	char *sig;
} VIRUS;

typedef struct LINK LINK;

struct LINK
{
	LINK *nextVirus;
	VIRUS *vir;
};

typedef struct fun_desc
{
	char *name;
	LINK *(*func)(LINK *);
} FunDesc;

//VIRUS rule of 5
void VIRUSctor(VIRUS *curVirus, unsigned short sigSize, char virusName[], char *sig);
void VIRUSdtor(VIRUS *curVirus);
VIRUS *VIRUScreate(unsigned short sigSize, char virusName[], char *sig);
void VIRUSdestroy(VIRUS *curVirus);

//antivirus functions
VIRUS *readVirus(FILE *virusFile);
void detect_virus(char *buffer, unsigned int size, LINK *virus_list);
void printVirus(VIRUS *curVirus, FILE *output);
void printSignature(VIRUS *curVirus, FILE *output);
void printHex(char *buffer, int length, FILE *output);
void printDetection(int location, VIRUS *data);

//linked list functions
void list_print(LINK *virus_list, FILE *output);
LINK *list_append(LINK *virus_list, VIRUS *data);
void list_free(LINK *virus_list);

//menu functions
LINK *loadSignatures(LINK *virusList);
LINK *printSignatures(LINK *virusList);
LINK *detectViruses(LINK *virusList);
LINK *quit(LINK *virusList);

bool menuRangeCheck(int index, int menu_size);
void processArgs(int argc, char **argv);

char *file_to_check = NULL;

int main(int argc, char **argv)
{
	int menu_size = 0, funcIndex = 0;
	char input[BUF_SIZE], inputTmp[BUF_SIZE];
	LINK *virusList = NULL;
	
	processArgs(argc, argv);

	FunDesc menu[] = {{"Load signatures", &loadSignatures},
					  {"Print signatures", &printSignatures},
					  {"Detect viruses", &detectViruses},
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
			printf("Not within Bounds.\n");
			(*quit)(virusList);
		}
	}
	return 0;
}

//main functions
bool menuRangeCheck(int index, int menu_size)
{
	return index >= 0 && index < menu_size;
}

void processArgs(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], "-i", 2) == 0)
		{
			file_to_check = argv[++i];
		}
	}
}

//menu functions
LINK *loadSignatures(LINK *virusList)
{
	char filenameTmp[BUF_SIZE], filename[BUF_SIZE];
	FILE *virusFile = NULL;
	VIRUS *curVirus = NULL;
	LINK *list = virusList;

	fgets(filenameTmp, BUF_SIZE, stdin);
	sscanf(filenameTmp, "%s", filename);
	virusFile = fopen(filename, "r");

	while (virusFile != NULL && fgetc(virusFile) != EOF)
	{
		fseek(virusFile, -1, SEEK_CUR);
		curVirus = readVirus(virusFile);
		list = list_append(list, curVirus);
	}
	if (virusFile == NULL)
	{
		printf("Could not open file.\n");
	}
	else
	{
		fclose(virusFile);
	}
	return list;
}

LINK *printSignatures(LINK *virusList)
{
	if (virusList != NULL)
	{
		list_print(virusList, stdout);
	}
	return virusList;
}

LINK *detectViruses(LINK *virusList)
{
	char fileBuffer[FILE_BUF_SIZE];
	int readSize = FILE_BUF_SIZE;
	FILE *file = fopen(file_to_check, "r");

	if (file != NULL)
	{
		fseek(file, 0L, SEEK_END);
		if (ftell(file) < readSize)
		{
			readSize = ftell(file);
		}
		fseek(file, 0L, SEEK_SET);

		fread(fileBuffer, sizeof(char), readSize, file);
		fclose(file);

		detect_virus(fileBuffer, readSize, virusList);
	}
	else
	{
		printf("Could not open the file.\n");
	}
	return virusList;
}

LINK *quit(LINK *virusList)
{
	list_free(virusList);
	virusList = NULL;
	exit(0);
}

//antivirus functions
VIRUS *readVirus(FILE *virusFile)
{
    VIRUS * virus = malloc(sizeof(VIRUS));

    fread(virus, sizeof(char), SIG_SIZ_LEN + VIR_NAM_LEN, virusFile);
    
    virus->sig = malloc(virus->sigSize * sizeof(char));
    fread(virus->sig, sizeof(char), virus->sigSize, virusFile);

    return virus;
}

void detect_virus(char *buffer, unsigned int size, LINK *virus_list)
{
	VIRUS *curVirus = NULL;
	LINK *curLink = NULL;

	for (int i = 0; i < size; i++)
	{
		curLink = virus_list;
		while (curLink != NULL)
		{
			curVirus = curLink->vir;
			if (i + curVirus->sigSize < size && memcmp(buffer + i, curVirus->sig, curVirus->sigSize) == 0)
			{
				printDetection(i, curVirus);
			}
			curLink = curLink->nextVirus;
		}
	}
}

void printVirus(VIRUS *curVirus, FILE *output)
{
	fprintf(output, "VIRUS name: %s\nVirus size: %d\nSignature:\n", curVirus->virusName, curVirus->sigSize);
	printHex(curVirus->sig, curVirus->sigSize, output);
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

void printDetection(int location, VIRUS *data)
{
	printf("Starting location: %d\n", location);
	printf("VIRUS name: %s\nSignature size: %d\n", data->virusName, data->sigSize);
}

//curVirus rule of 3
void VIRUSctor(VIRUS *curVirus, unsigned short sigSize, char virusName[], char *sig)
{
	curVirus->sigSize = sigSize;
	curVirus->sig = sig;

	for (int i = 0; i < 16; i++)
	{
		curVirus->virusName[i] = virusName[i];
	}
}

VIRUS *VIRUScreate(unsigned short sigSize, char virusName[], char *sig)
{
	VIRUS *curVirus = NULL;

	curVirus = malloc(sizeof(VIRUS));
	VIRUSctor(curVirus, sigSize, virusName, sig);
	return curVirus;
}

void VIRUSdtor(VIRUS *curVirus)
{
	free(curVirus->sig);
	curVirus->sig = NULL;
}

void VIRUSdestroy(VIRUS *curVirus)
{
	VIRUSdtor(curVirus);
	free(curVirus);
}

//linked list creator
void LINKctor(LINK *link, VIRUS *data)
{
	link->nextVirus = NULL;
	link->vir = data;
}

LINK *LINKcreate(VIRUS *data)
{
	LINK *newLink = malloc(sizeof(LINK));
	newLink->nextVirus = NULL;
	newLink->vir = data;
	return newLink;
}

//linked list functions
void list_print(LINK *virus_list, FILE *output)
{
	if (virus_list != NULL)
	{
		list_print(virus_list->nextVirus, output);
		printVirus(virus_list->vir, output);
	}
}

LINK *list_append(LINK *virus_list, VIRUS *data)
{
	LINK *newLink = LINKcreate(data);
	newLink->nextVirus = virus_list;
	return newLink;
}

void list_free(LINK *virus_list)
{
	if (virus_list != NULL)
	{
		VIRUSdestroy(virus_list->vir);
		list_free(virus_list->nextVirus);
		free(virus_list);
		virus_list = NULL;
	}
}