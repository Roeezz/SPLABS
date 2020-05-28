#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LOW_CHAR 0x20
#define HIGH_CHAR 0x7E
#define NULL_CHAR '\0'
#define INITIAL_SIZE 5
#define BUF_SIZE 128

typedef struct state
{
	char debug_mode;
	char file_name[128];
	int unit_size;
	unsigned char mem_buf[10000];
	size_t mem_count;

	int ext_stat;
} state;

typedef struct fun_desc
{
	char *desc;
	void (*func)(state *);
} FunDesc;

bool menuRangeCheck(int index, int menu_size);
void printDebug(state *prgmState);
void printMenu(FunDesc menu[]);

void toggelDebug(state *prgmState);
void setUnitSize(state *prgmState);
void setFileName(state *prgmState);
void quit(state *prgmState);

int main(int argc, char **argv)
{
	int funcIndex = 0, menuSize = 0;
	char input[BUF_SIZE], tmpBuf[BUF_SIZE];
	state *prgmState = (state *)(calloc(1, sizeof(state)));
	FunDesc menu[] = {{"Toggle Degbug Mode", &toggelDebug},
					  {"Set File Name", &setFileName},
					  {"Set Unit Size", &setUnitSize},
					  {"Quit", &quit},
					  {NULL, NULL}};

	for (int i = 0; menu[i].func != NULL; i++)
	{
		menuSize++;
	}
	while (true)
	{
		if (prgmState->debug_mode)
			printDebug(prgmState);
		printMenu(menu);

		printf("Please choose an action by number: ");
		fgets(tmpBuf, BUF_SIZE, stdin);
		sscanf(tmpBuf, "%s", input);
		funcIndex = atoi(input);
		if (menuRangeCheck(funcIndex, menuSize))
		{
			(*menu[funcIndex].func)(prgmState);
		}
		else
		{
			printf("Not within Bounds\n");
			prgmState->ext_stat = 1;
			(*quit)(prgmState);
		}
	}
	(*quit)(prgmState);
	return 0;
}

bool menuRangeCheck(int index, int menu_size)
{
	return index >= 0 && index < menu_size;
}

void printDebug(state *prgmState)
{
	fprintf(stderr, "DEBUG PRINT:\n");
	fprintf(stderr, "-unit_size: %d\n", prgmState->unit_size);
	fprintf(stderr, "-file_name: %s\n", prgmState->file_name);
	fprintf(stderr, "-mem_count: %zd\n", prgmState->mem_count);
}

void printMenu(FunDesc menu[])
{
	printf("Actions menu:\n");
	for (int i = 0; menu[i].desc != NULL; i++)
	{
		printf("%i-%s\n", i, menu[i].desc);
	}
}

void toggelDebug(state *prgmState)
{
	prgmState->debug_mode = ~prgmState->debug_mode;
}
void setFileName(state *prgmState)
{
	printf("Input a file name: ");
	char tmpBuf[BUF_SIZE];
	fgets(tmpBuf, BUF_SIZE, stdin);
	sscanf(tmpBuf, "%s", prgmState->file_name);
	if (prgmState->debug_mode)
	{
		fprintf(stderr, "Debug: file name set to %s\n", prgmState->file_name);
	}
}

void setUnitSize(state *prgmState)
{
	printf("Input a unit size: ");
	char tmpBuf[BUF_SIZE];
	int unitSize = 0;
	fgets(tmpBuf, BUF_SIZE, stdin);
	sscanf(tmpBuf, "%d", &unitSize);
	if (unitSize == 1 || unitSize == 2 || unitSize == 4)
	{
		if (prgmState->debug_mode)
		{
			fprintf(stderr, "Debug: set size to %d\n", unitSize);
		}
		prgmState->unit_size = unitSize;
	}
	else
	{
		fprintf(stderr, "Set Unit Size: Invalid unit size: %d\n", unitSize);
	}
}

void loadFile(state *prgmState)
{
	FILE *file = NULL;
	char tmpBuf[BUF_SIZE], input[BUF_SIZE];
	int location = 0, length = 0;

	if (!prgmState->file_name)
	{
		fprintf(stderr, "Load File Into Memory: file name is null\n");
		return;
	}
	if (!(file = fopen(prgmState->file_name)))
	{
		perror("fopen");
		return;
	}
	printf("Please enter <location> <length>\n");
	fgets(tmpBuf, BUF_SIZE, stdin);
	sscanf(tmpBuf, "%d", input);
}

void quit(state *prgmState)
{
	int exitStatus = prgmState->ext_stat;
	free(prgmState);
	printf("Exiting. status: %d\n", exitStatus);
	exit(exitStatus);
}

