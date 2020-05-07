#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LOW_CHAR 0x20
#define HIGH_CHAR 0x7E

bool rangeCheck(char c)
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
	if (rangeCheck(c))
	{
		out = c + 3;
	}

	return out;
}

char decrypt(char c)
{
	char out = c;
	if (rangeCheck(c))
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
	if (rangeCheck(c))
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

char *map(char *array, int array_length, char (*f)(char))
{
	char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
	for (int i = 0; i < array_length; i++)
	{
		mapped_array[i] = (*f)(array[i]);
	}
	return mapped_array;
}

int main(int argc, char **argv)
{
	int base_len = 5;
	char arr1[base_len];
	char *arr2 = map(arr1, base_len, my_get);
	char *arr3 = map(arr2, base_len, quit);
	char *arr4 = map(arr3, base_len, encrypt);
	char *arr5 = map(arr4, base_len, dprt);
	char *arr6 = map(arr5, base_len, decrypt);
	char *arr7 = map(arr6, base_len, cprt);
	free(arr2);
	free(arr3);
	free(arr4);
	free(arr5);
	free(arr6);
}