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

void processCmdLine(cmdLine *pCmdLine);
void handleExit(cmdLine *pCmdLine, int exitCode);
void handleCD(cmdLine *pCmdLine);

void execute(cmdLine *pCmdLine);
void waitForChild(int pid);
void debugger();

bool _debug = false;
int _childPID = 0;
char *_curCmd = NULL;

int main(int argc, char **argv)
{
    char input[BUF_SIZE];
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
            handleExit(pCmdLine, 1);
        }
    }

    while (true)
    {
        printf("myshell:%s# ",getcwd(NULL, PATH_MAX));
        fgets(input, BUF_SIZE, stdin);
        pCmdLine = parseCmdLines(input);
        processCmdLine(pCmdLine);
    }
}

void processCmdLine(cmdLine *pCmdLine)
{
    _curCmd = pCmdLine->arguments[0];
    if (strcmp(_curCmd, "quit") == 0 || strcmp(_curCmd, "exit") == 0)
    {
        handleExit(pCmdLine, 0);
    }
    else if (strcmp(_curCmd, "cd") == 0)
    {
        handleCD(pCmdLine);
    }
    else
    {
        execute(pCmdLine);
    }
}

void execute(cmdLine *pCmdLine)
{
    if (!(_childPID = fork()))
    {
        execvp(_curCmd, pCmdLine->arguments);
        perror("Could not execute command");
        handleExit(pCmdLine, 1);
    }
    debugger();
    if (pCmdLine->blocking)
    {
        waitForChild(_childPID);
    }
}

void handleExit(cmdLine *pCmdLine, int exitCode)
{
    freeCmdLines(pCmdLine);
    exit(exitCode);
}

void handleCD(cmdLine *pCmdLine)
{
    int result = chdir(pCmdLine->arguments[1]);
    if (result == -1)
    {
        perror("Could not change directory");
    }
}

void waitForChild(int pid)
{
    waitpid(pid, NULL, 0);
}

void debugger()
{
    if (_debug)
    {
        fprintf(stderr, "Currnet command: %s\nProccess ID: %i\n", _curCmd, _childPID);
    }
}