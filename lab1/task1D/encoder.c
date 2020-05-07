#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void processDebug(bool *debug, const int argc, char *argv[]);
void processEncrypt(char *argument, bool *encrypt, int *sign);

void uppercase(FILE *input, FILE *output, bool debug);
void encryption(FILE *input, FILE *output, bool debug, int sign, char *keyArg);
void debugger(char origin, char decoded);

int main(int argc, char *argv[])
{
	FILE *input = stdin;
	FILE *output = stdout;
	char *argument;
	bool debug = false, encrypt = false;
	int sign = 0, keyIndex;

	for (int i = 1; i < argc; i++)
	{
		argument = argv[i];
		if (strncmp(argument, "-D", 2) == 0)
		{
			processDebug(&debug, argc, argv);
		}
		else if (strncmp(argument, "+e", 2) == 0 || strncmp(argument, "-e", 2) == 0)
		{
			keyIndex = i;
			processEncrypt(argument, &encrypt, &sign);
		}
		else if (strncmp(argument, "-o", 2) == 0)
		{
			output = fopen(argument + 2, "w");
		}
		else
		{
			printf("Invalid program argument: %s", argument);
			return 1;
		}
	}
	if (encrypt)
	{
		encryption(input, output, debug, sign, argv[keyIndex]);
	}
	else
	{
		uppercase(input, output, debug);
	}
	fclose(input);
	fclose(output);
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

void processEncrypt(char *argument, bool *encrypt, int *sign)
{
	*encrypt = true;

	if (argument[0] == '+')
	{
		*sign = 1;
	}
	else if (argument[0] == '-')
	{
		*sign = -1;
	}
}

void uppercase(FILE *input, FILE *output, bool debug)
{
	int origin = 0, decoded = 0;
	int caseBottom = 97, caseTop = 122, modificator = 32;

	while ((origin = fgetc(input)) != EOF)
	{
		decoded = origin;
		if (caseBottom <= origin && origin <= caseTop)
		{
			decoded = origin - modificator;
		}
		if (debug)
		{
			debugger(origin, decoded);
		}

		fputc(decoded, output);
	}
}

void encryption(FILE *input, FILE *output, bool debug, int sign, char *keyArg)
{
	int origin = 0, decoded = 0, keyIndex = 0;

	while ((origin = fgetc(input)) != EOF)
	{
		decoded = origin;
		if (origin != '\n')
		{
			int modification = (keyArg[keyIndex + 2] - '0') * sign;
			decoded = origin + modification;
			keyIndex = (keyIndex + 1) % (strlen(keyArg) - 2);
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