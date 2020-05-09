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

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

//cmd line functions
void printCWD();
void processCmdLine(process **process_list, cmdLine *pCmdLine);
void handleExit(process **process_list, int exitCode);
void handleCD(cmdLine *pCmdLine);
void handleExecute(process **process_list, cmdLine *pCmdLine);

//process functions
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void printProcessList(process **process_list);
char *getStatusName(int status);
void printArgs(process *proc);
void freeProcessList(process **process_list);
void updateProcessList(process **process_list);
void chooseUpdateAction(int wstatus, process *proc);
void updateProcessStatus(process *process_list, int pid, int status);
void cleanTerminated(process **process_list);
void removeProcess(process **process_list, process *prevProc, process *proc);

//process rule of 5
process *processCREATE(cmdLine *cmd, pid_t pid, int status, process *next);
void processCtor(process *this, cmdLine *cmd, pid_t pid, int status, process *next);
void processDESTROY(process *proc);
void processDtor(process *this);

//exection functions
void execute(cmdLine *pCmdLine);
void waitForChild(int pid);

void freeLine(cmdLine *pCmdLine);

//debugging
void debugger();

//global vars
bool _debug = false; //modified only in main()

int main(int argc, char **argv)
{
    char input[BUF_SIZE];
    process **process_list = calloc(1, sizeof(process *));

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0)
        {
            _debug = true;
        }
        else
        {
            printf("Invalid argument: %s\n", argv[i]);
            handleExit(process_list, 1);
        }
    }

    while (true)
    {
        printCWD();
        fgets(input, BUF_SIZE, stdin);
        processCmdLine(process_list, parseCmdLines(input));
    }
}

void printCWD()
{
    char *cwd = getcwd(NULL, PATH_MAX);
    printf("myshell:%s#> ", cwd);
    free(cwd);
}

void processCmdLine(process **process_list, cmdLine *pCmdLine)
{
    char *command = pCmdLine->arguments[0];
    if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)
    {
        freeLine(pCmdLine);
        handleExit(process_list, 0);
    }
    else if (strcmp(command, "cd") == 0)
    {
        handleCD(pCmdLine);
    }
    else if (strcmp(command, "procs") == 0)
    {
        printProcessList(process_list);
    }
    else
    {
        handleExecute(process_list, pCmdLine);
        return; //we return so that after executing we don't free the cmdLine
    }
    freeLine(pCmdLine);
}

void handleExecute(process **process_list, cmdLine *pCmdLine)
{
    int pid = 0;
    if (!(pid = fork()))
    {
        execute(pCmdLine);
        perror("Could not execute command");
        freeLine(pCmdLine);
        handleExit(process_list, 1);
    }
    debugger(pCmdLine->arguments[0], pid);
    if (pCmdLine->blocking)
    {
        waitForChild(pid);
        freeLine(pCmdLine);
    }
    else
    {
        addProcess(process_list, pCmdLine, pid);
    }
}
void execute(cmdLine *pCmdLine)
{
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

void waitForChild(int pid)
{
    waitpid(pid, NULL, 0);
}

void handleExit(process **process_list, int exitCode)
{
    if (process_list != NULL)
        freeProcessList(process_list);
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

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    if (process_list == NULL)
    {
        printf("Invalid process list");
        return;
    }
    else if (*process_list == NULL)
    {
        *process_list = processCREATE(cmd, pid, RUNNING, NULL);
    }
    else
    {
        addProcess(&(*process_list)->next, cmd, pid);
    }
}

void printProcessList(process **process_list)
{
    puts("|INDEX   PID     STATUS            COMMAND: ARGUMETS");
    process *proc = *process_list;
    char *command = NULL, *status = NULL;
    int index = 1;

    updateProcessList(process_list);

    while (proc != NULL)
    {
        command = proc->cmd->arguments[0];
        status = getStatusName(proc->status);
        printf("|%d)     %d  |  %s     |     %s: ", index, proc->pid, status, command);
        printArgs(proc);
        puts("");
        proc = proc->next;
        index++;
    }
    printf("|___________________________________________________\n");
    cleanTerminated(process_list);
}

void cleanTerminated(process **process_list)
{
    process *proc = *process_list;
    process *prevProc = NULL, *nextProc;
    while (proc != NULL)
    {
        nextProc = proc->next;
        if (proc->status == TERMINATED)
        {
            removeProcess(process_list, prevProc, proc);
        }
        else
        {
            prevProc = proc;
        }
        proc = nextProc;
    }
}

void removeProcess(process **process_list, process *prevProc, process *proc)
{
    if (prevProc == NULL)
    {
        *process_list = proc->next;
    }
    else
    {
        prevProc->next = proc->next;
    }
    processDtor(proc);
    free(proc);
}

void printArgs(process *proc)
{
    char *const *argList = proc->cmd->arguments;
    cmdLine *cmd = proc->cmd;
    if (cmd->argCount == 1)
    {
        printf("(none)");
        return;
    }
    for (int i = 1; i < cmd->argCount - 1; i++)
    {
        printf(" %s,", argList[i]);
    }
    printf(" %s", argList[cmd->argCount - 1]);
}

char *getStatusName(int status)
{
    switch (status)
    {
    case RUNNING:
        return "Running";
    case SUSPENDED:
        return "Suspended";
    case TERMINATED:
        return "Terminated";
    default:
        return "???";
    }
}

void freeProcessList(process **process_list)
{
    processDESTROY(*process_list);
    free(process_list);
}

void updateProcessList(process **process_list)
{
    int wstatus = 0;
    process *proc = *process_list;
    int waitResult = 0;
    while (proc)
    {
        wstatus = 0;
        waitResult = waitpid(proc->pid, &wstatus, WNOHANG);
        if (waitResult > 0)
        {
            chooseUpdateAction(wstatus, proc);
        }
        else if (waitResult < 0)
        {
            perror("An error has occured while waiting for process");
        }
        proc = proc->next;
    }
}

void chooseUpdateAction(int wstatus, process *proc)
{
    if (WIFSTOPPED(wstatus))
    {
        updateProcessStatus(proc, proc->pid, SUSPENDED);
    }
    else if (WIFCONTINUED(wstatus))
    {
        updateProcessStatus(proc, proc->pid, RUNNING);
    }
    else if (WIFSIGNALED(wstatus) || WIFEXITED(wstatus))
    {
        updateProcessStatus(proc, proc->pid, TERMINATED);
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *proc = process_list;
    while (proc && proc->pid != pid)
    {
        proc = proc->next;
    }
    proc->status = status;
}

process *processCREATE(cmdLine *cmd, pid_t pid, int status, process *next)
{
    process *proc = malloc(sizeof(process));
    processCtor(proc, cmd, pid, status, next);
    return proc;
}

void processCtor(process *this, cmdLine *cmd, pid_t pid, int status, process *next)
{
    this->cmd = cmd;
    this->pid = pid;
    this->status = status;
    this->next = next;
}

void processDESTROY(process *proc)
{
    if (proc != NULL)
    {
        processDESTROY(proc->next);
        processDtor(proc);
        free(proc);
    }
}

void processDtor(process *this)
{
    freeLine(this->cmd);
}

void debugger(char *command, int pid)
{
    if (_debug)
    {
        fprintf(stderr, "Currnet command: %s\nProccess ID: %i\n", command, pid);
    }
}

void freeLine(cmdLine *pCmdLine)
{
    freeCmdLines(pCmdLine);
}