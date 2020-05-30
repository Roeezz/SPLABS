#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

typedef struct state
{
	char debug_mode;
	char display_mode;
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

typedef struct PrintForm
{
	const char *format;
	long data;
} PrintForm;

bool menuRangeCheck(int index, int menu_size);
void printMenu(FunDesc menu[]);
void newline(FILE *stream);

void toggelDebug(state *prgmState);
void setUnitSize(state *prgmState);
void setFileName(state *prgmState);
void loadFile(state *prgmState);
void toggleDisplay(state *prgmState);
void quit(state *prgmState);

int main(int argc, char **argv)
{
	int funcIndex = 0, menuSize = 0;
	char input[BUF_SIZE], tmpBuf[BUF_SIZE];
	state *prgmState = (calloc(1, sizeof(state)));
	prgmState->unit_size = 1;
	FunDesc menu[] = {{"Toggle Degbug Mode", &toggelDebug},
					  {"Set File Name", &setFileName},
					  {"Set Unit Size", &setUnitSize},
					  {"Load Into Memory", &loadFile},
					  {"Toggle Display Mode", &toggleDisplay},
					  {"Quit", &quit},
					  {NULL, NULL}};

	for (int i = 0; menu[i].func != NULL; i++)
	{
		menuSize++;
	}
	while (true)
	{
		printDebug(prgmState,
				   "-unit_size: %d\n-file_name: %s\n-mem_count: %zd\n-display_mode: %d\n",
				   prgmState->unit_size,
				   prgmState->file_name,
				   prgmState->mem_count,
				   prgmState->display_mode);
		printMenu(menu);

		printf("Please choose an action by number: ");
		fgets(tmpBuf, BUF_SIZE, stdin);
		sscanf(tmpBuf, "%s", input);
		funcIndex = atoi(input);
		newline(stdin);
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

void toggelDebug(state *prgmState)
{
	prgmState->debug_mode = ~prgmState->debug_mode;
	prgmState->debug_mode ? puts("Debug flag now on, printing debug messages") : puts("Debug flag now off, you're on your own");
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

void toggleDisplay(state *prgmState)
{
	prgmState->display_mode = ~prgmState->display_mode;
	prgmState->display_mode ? puts("Display flag now on, hexadecimal representation") : puts("Display flag now off, decimal representation");
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
		printDebug(prgmState, "Set size to %d\n", prgmState->unit_size);
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
	char tmpBuf[BUF_SIZE];
	int location = 0, length = 0;

	if (!prgmState->file_name)
	{
		fprintf(stderr, "Load File Into Memory: file name is null\n");
		return;
	}
	if (!(file = fopen(prgmState->file_name, "r")))
	{
		perror("fopen");
		return;
	}

	printf("Please enter location: ");
	fgets(tmpBuf, BUF_SIZE, stdin);
	sscanf(tmpBuf, "%x", &location);

	printf("Please enter length: ");
	fgets(tmpBuf, BUF_SIZE, stdin);
	sscanf(tmpBuf, "%d", &length);
	printDebug(prgmState,
			   "Filename: %s\nLocations: %x\nLength: %d\n",
			   prgmState->file_name,
			   location,
			   length);

	fseek(file, location, SEEK_SET);
	fread(prgmState->mem_buf, prgmState->unit_size, length, file);
	prgmState->mem_count += length * prgmState->unit_size;
	fclose(file);
	printf("Loaded %d units into memory\n", length);
}

void quit(state *prgmState)
{
	int exitStatus = prgmState->ext_stat;
	free(prgmState);
	printf("Exiting. status: %d\n", exitStatus);
	exit(exitStatus);
}

void newline(FILE *stream)
{
	fprintf(stream, "\n");
}