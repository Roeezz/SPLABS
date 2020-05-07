#include "LineParser.h"
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 2048

void execute(cmdLine *pCmdLine);

int main(int argc, char **argv)
{

    char curDir[PATH_MAX], input[BUF_SIZE];
    cmdLine *pCmdLine = NULL;

    getcwd(curDir, PATH_MAX);
    while (true)
    {
        puts(curDir);
        fgets(input, BUF_SIZE, stdin);
        if (strncmp(input, "quit", 4) == 0)
        {
            freeCmdLines(pCmdLine);
            exit(0);
        }
        pCmdLine = parseCmdLines(input);
        execute(pCmdLine);
    }
}

void execute(cmdLine *pCmdLine)
{
    execv(pCmdLine->arguments[0], pCmdLine->arguments);
    perror("Could not execute the command");
    freeCmdLines(pCmdLine);
    exit(1);
}
