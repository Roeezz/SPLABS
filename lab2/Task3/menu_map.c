#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LOW_CHAR 0x20
#define HIGH_CHAR 0x7E
#define NULL_CHAR '\0'
#define INITIAL_SIZE 5

bool menuRangeCheck(int index, int menu_size);
bool charRangeCheck(char c);
char censor(char c);
char encrypt(char c);
char decrypt(char c);
char dprt(char c);
char cprt(char c);
char my_get(char c);
char quit(char c);
char *map(char *array, int array_length, char (*func)(char));

typedef struct fun_desc
{
	char *desc;
	char (*func)(char);
} FunDesc;

int main(int argc, char **argv)
{
	int menu_size = 0, funcIndex = 0, input_size = 50;
	char *carray = (char *)malloc(INITIAL_SIZE * sizeof(char)), *carrayTmp, input[input_size], inputTmp[input_size];
	*carray = NULL_CHAR;

	FunDesc menu[] = {{"Censor '!' chars in the string", censor},
					  {"Encrypt a string", &encrypt},
					  {"Decrypt a string", &decrypt},
					  {"Print decimal value of string", &dprt},
					  {"Print string", &cprt},
					  {"Input a string", &my_get},
					  {"Quit the program", &quit},
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
			printf("%i)   %s\n", i, menu[i].desc);
		}

		printf("Please choose an action by number: ");
		fgets(input, input_size, stdin);
		sscanf(input, "%s", inputTmp);
		funcIndex =  atoi(inputTmp);
		if (menuRangeCheck(funcIndex, menu_size))
		{
			printf("Within Bounds\n");
			carrayTmp = map(carray, INITIAL_SIZE, menu[funcIndex].func);
			free(carray);
			carray = carrayTmp;
			printf("DONE.\n\n");
		}
		else
		{
			printf("Not within Bounds\n");
			free(carray);
			(*quit)('q');
		}
	}

	free(carray);
	return 0;
}

bool menuRangeCheck(int index, int menu_size)
{
	return index > 0 && index <= menu_size;
}

bool charRangeCheck(char c)
{
	return c >= LOW_CHAR && c <= HIGH_CHAR;
}

char censor(char c)
{
	if (c == '!')
		return '.';
	else
		return c;
}

char encrypt(char c)
{
	char out = c;
	if (charRangeCheck(c))
	{
		out = c + 3;
	}

	return out;
}

char decrypt(char c)
{
	char out = c;
	if (charRangeCheck(c))
	{
		out = c - 3;
	}

	return out;
}

char dprt(char c)
{
	printf("%i\n", c);
	return c;
}

char cprt(char c)
{
	if (charRangeCheck(c))
	{
		printf("%c\n", c);
	}
	else
	{
		printf("%c\n", '.');
	}
	return c;
}

char my_get(char c)
{
	char out = fgetc(stdin);
	return out;
}

char quit(char c)
{
	if (c == 'q')
	{
		exit(0);
	}
	return c;
}

char *map(char *array, int array_length, char (*func)(char))
{
	char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
	for (int i = 0; i < array_length; i++)
	{
		mapped_array[i] = (*func)(array[i]);
	}
	return mapped_array;
}