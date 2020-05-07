#include "LineParser.h"
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 2048

void execute(cmdLine *pCmdLine);
void waitForChild(int pid);
void debugger();

bool _debug = false;
int _childPID = 0;
char *_currCommand = NULL;

int main(int argc, char **argv)
{
    char curDir[PATH_MAX], input[BUF_SIZE], inputCheck[BUF_SIZE];
    cmdLine *pCmdLine = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0)
        {
            _debug = true;
        }
        else
        {
            printf("Invalid argument: %s\n", argv[i]);
            exit(1);
        }
    }

    getcwd(curDir, PATH_MAX);
    while (true)
    {
        printf("%s$ ", curDir);
        fgets(input, BUF_SIZE, stdin);
        sscanf(input, "%s", inputCheck);
        if (strcmp(inputCheck, "quit") == 0 || strcmp(inputCheck, "exit") == 0)
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
    _currCommand = pCmdLine->arguments[0];
    if (!(_childPID = fork()))
    {
        execvp(_currCommand, pCmdLine->arguments);
        perror("Could not execute the command");
        freeCmdLines(pCmdLine);
        exit(1);
    }
    debugger();
    waitForChild(_childPID);
}

void waitForChild(int pid)
{
    waitpid(pid, NULL, 0);
}

void debugger()
{
    if (_debug)
    {
        fprintf(stderr, "Currnet command: %s\nProccess ID: %i\n", _currCommand, _childPID);
    }
}