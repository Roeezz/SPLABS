#include <stdlib.h>
#include <stdio.h>

void PrintHex(char *buffer, int length);

int main(int argc, char **argv)
{
    FILE *input = NULL;
    int buf_size = 0;
    char *byteBuffer = NULL;

    if ((input = fopen(argv[1], "r")))
    {
        fseek(input, 0L, SEEK_END);
        buf_size = ftell(input);
        fseek(input, 0L, SEEK_SET);
        byteBuffer = (char *)calloc(buf_size, sizeof(char));

        fread(byteBuffer, sizeof(char), buf_size, input);

        PrintHex(byteBuffer, buf_size);

        free(byteBuffer);
        fclose(input);
    }
    else
    {
        printf("Could not open file\n");
    }
    return 0;
}

void PrintHex(char *buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i] & 0xFF);
    }
    printf("\n");
}