#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void processDebug(bool *debug, const int argc, char *argv[]);

void uppercase(FILE *input, FILE *output, bool debug);
void encryption(FILE *input, FILE *output, bool debug, int sign, char *key);
void debugger(char origin, char decoded);

int main(int argc, char *argv[])
{
	FILE *input = stdin;
	FILE *output = stdout;
	char *argument = NULL, *key = NULL;
	bool debug = false, encrypt = false;
	int sign = 0;

	for (int i = 1; i < argc; i++)
	{
		argument = argv[i];
		if (strncmp(argument, "-D", 2) == 0)
		{
			processDebug(&debug, argc, argv);
		}
		else
		{
			printf("Invalid program argument: %s", argument);
		}
	}
	uppercase(input, output, debug);
	return 0;
}

void processDebug(bool *debug, const int argc, char *argv[])
{
	*debug = true;
	for (int j = 1; j < argc; j++)
	{
		fprintf(stderr, "%s\n", argv[j]);
	}
}

void uppercase(FILE *input, FILE *output, bool debug)
{
	int origin, decoded;

	while ((origin = fgetc(input)) != EOF)
	{
		decoded = origin;

		if (97 <= origin && origin <= 122)
		{
			decoded = origin - 32;
		}
		if (debug)
		{
			debugger(origin, decoded);
		}

		fputc(decoded, output);
	}
}

void debugger(char origin, char decoded)
{
	if (origin == '\n')
	{
		fputc(origin, stderr);
	}
	else
	{
		fprintf(stderr, "%i   %i\n", origin, decoded);
	}
}