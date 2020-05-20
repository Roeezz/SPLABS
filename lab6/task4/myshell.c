#include "LineParser.h"
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 2048

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

#define REDCT_INPUT 0
#define REDCT_OUTPUT 1


typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

typedef struct myShellVar
{
    char *name;
    char *value;
    struct myShellVar *next;
} myShellVar;

//cmd line functions
void printCWD();
void processCmdLine(process **process_list, cmdLine *pCmdLine, myShellVar **varList);
void handleExit(process **process_list, myShellVar **varList, int exitCode);
void handleCD(cmdLine *pCmdLine);
void handleExecute(process **process_list, cmdLine *pCmdLine, myShellVar **varList);
void sendSignal(char *signame, pid_t pid);
void handleSet(cmdLine *pCmdLine, myShellVar **varList);
void freeLine(cmdLine *pCmdLine);

//process functions
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void printProcessList(process **process_list);
char *getStatusName(int status);
void printArgs(process *proc);
void freeProcessList(process **process_list);
void updateProcessList(process **process_list);
void chooseUpdateAction(int wstatus, process *proc);
void updateProcessStatus(process *process_list, pid_t pid, int status);
void cleanTerminated(process **process_list);
void removeProcess(process **process_list, process *prevProc, process *proc);

//process rule of 5
process *processCREATE(cmdLine *cmd, pid_t pid, int status, process *next);
void processCtor(process *this, cmdLine *cmd, pid_t pid, int status, process *next);
void processDESTROY(process *proc);
void processDtor(process *this);

//var functions
void addMyShellVar(char *name, char *value, myShellVar **varList);
void freeVarList(myShellVar **varList);
myShellVar *getVar(const char *name, myShellVar **varList);
void printVarList(myShellVar **varList);
int replaceMyShellVars(cmdLine *pCmdLine, myShellVar **varList);
void replaceIORedirect(cmdLine *pCmdLine, myShellVar **varList);

//myShellVar rule of 5
myShellVar *myShellVarCREATE(char *name, char *value, myShellVar *next);
void myShellVarCtor(myShellVar *var, char *name, char *value, myShellVar *next);
void myShellVarDESTROY(myShellVar *var);
void myShellVarDtor(myShellVar *var);
void myShellVarSetVal(myShellVar *var, char *value);

//exection functions
void execute(cmdLine *pCmdLine);
int executePipes(process **process_list, cmdLine *pCmdLine, myShellVar **varList, int readfd);
void setupPipe(cmdLine *pCmdline, int writefd, int readfd);
int redirectIO(cmdLine *pCmdLine);
pid_t waitForChild(pid_t pid);

//string functions
char *strClone(char *str);
char *join(char *const *strArr, int start, int end, const char *seperator);
char *concat(char *str1, char *str2);

//debugging
void debugger();

//global vars
bool _debug = false; //modified only in main()

int main(int argc, char **argv)
{
    char input[BUF_SIZE];
    process **process_list = calloc(1, sizeof(process *));
    *process_list = NULL;
    myShellVar **varList = calloc(1, sizeof(myShellVar *));
    *varList = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0)
        {
            _debug = true;
        }
        else
        {
            printf("Invalid argument: %s\n", argv[i]);
            handleExit(process_list, varList, 1);
        }
    }

    while (true)
    {
        printCWD();
        fgets(input, BUF_SIZE, stdin);
        processCmdLine(process_list, parseCmdLines(input), varList);
    }
}

void printCWD()
{
    char *cwd = getcwd(NULL, PATH_MAX);
    printf("myshell:%s#> ", cwd);
    free(cwd);
}

void processCmdLine(process **process_list, cmdLine *pCmdLine, myShellVar **varList)
{
    char *command = pCmdLine->arguments[0];
    if (strcmp(command, "quit") == 0)
    {
        freeLine(pCmdLine);
        handleExit(process_list, varList, 0);
    }
    else if (strcmp(command, "cd") == 0)
    {
        handleCD(pCmdLine);
    }
    else if (strcmp(command, "procs") == 0)
    {
        printProcessList(process_list);
    }
    else if (strcmp(command, "kill") == 0 || strcmp(command, "suspend") == 0 || strcmp(command, "wake") == 0)
    {
        sendSignal(command, atoi(pCmdLine->arguments[1]));
    }
    else if (strcmp(command, "set") == 0)
    {
        handleSet(pCmdLine, varList);
    }
    else if (strcmp(command, "vars") == 0)
    {
        printVarList(varList);
    }
    else
    {
        handleExecute(process_list, pCmdLine, varList);
        return; //we return so that after executing we don't free the cmdLine
    }
    freeLine(pCmdLine);
}

void handleExecute(process **process_list, cmdLine *pCmdLine, myShellVar **varList)
{
    pid_t pid = 0;
    int readfd = -1;
    cmdLine *cur = pCmdLine;
    while (cur->next != NULL)
    {
        if (replaceMyShellVars(cur, varList) < 0)
        {
            freeLine(pCmdLine);
            return;
        }
        readfd = executePipes(process_list, cur, varList, readfd);
        if (readfd < 0)
        {
            freeLine(pCmdLine);
            return;
        }
        cur = cur->next;
    }
    if (replaceMyShellVars(cur, varList) < 0)
    {
        freeLine(pCmdLine);
        return;
    }
    if (!(pid = fork()))
    {
        setupPipe(cur, -1, readfd);
        execute(cur);
        perror(pCmdLine->arguments[0]);
        freeLine(cur);
        handleExit(process_list, varList, 1);
    }
    if (cur->blocking)
    {
        waitForChild(pid);
        freeLine(pCmdLine);
    }
    else
    {
        addProcess(process_list, cur, pid);
    }
}

int executePipes(process **process_list, cmdLine *pCmdLine, myShellVar **varList, int readfd)
{
    pid_t pid;
    int pipefd[2], wstatus = 0;
    if (pipe(pipefd) < 0)
    {
        perror(pCmdLine->arguments[0]);
        return -1;
    }
    if (!(pid = fork()))
    {
        setupPipe(pCmdLine, pipefd[1], readfd);
        execute(pCmdLine);
        perror(pCmdLine->arguments[0]);
        freeLine(pCmdLine);
        handleExit(process_list, varList, 1);
    }
    close(pipefd[1]);
    wstatus = waitForChild(pid);
    if (WEXITSTATUS(wstatus) != 0)
    {
        return -1;
    }
    if (readfd != -1)
    {
        close(readfd);
    }
    return pipefd[0];
}

void setupPipe(cmdLine *pCmdline, int writefd, int readfd)
{
    if (readfd >= 0)
    {
        close(STDIN_FILENO);
        dup(readfd);
        close(readfd);
    }
    if (pCmdline->next != NULL)
    {
        close(STDOUT_FILENO);
        dup(writefd);
        close(writefd);
    }
}

void execute(cmdLine *pCmdLine)
{
    if (!redirectIO(pCmdLine))
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
}

pid_t waitForChild(pid_t pid)
{
    int wstatus = 0;
    waitpid(pid, &wstatus, 0);
    return wstatus;
}

int redirectIO(cmdLine *pCmdLine)
{
    int redirect = 0;
    if (pCmdLine->inputRedirect != NULL)
    {
        redirect = fclose(stdin);
        if (redirect < 0 || fopen(pCmdLine->inputRedirect, "r") == NULL)
            redirect = -1;
    }
    if (!redirect && pCmdLine->outputRedirect != NULL)
    {
        redirect = fclose(stdout);
        if (redirect < 0 || fopen(pCmdLine->outputRedirect, "w") == NULL)
            redirect = -1;
    }
    if (redirect < 0)
    {
        perror(pCmdLine->arguments[0]);
    }
    return redirect;
}

void handleExit(process **process_list, myShellVar **varList, int exitCode)
{
    freeProcessList(process_list);
    freeVarList(varList);
    exit(exitCode);
}

void handleCD(cmdLine *pCmdLine)
{
    char *path = pCmdLine->arguments[1];
    if (strncmp(path, "~", 1) == 0)
    {
        path = concat(getenv("HOME"), path + 1);
    }
    if (chdir(path) == -1)
    {
        perror(pCmdLine->arguments[0]);
    }
    free(path);
}

void handleSet(cmdLine *pCmdLine, myShellVar **varList)
{
    if (pCmdLine->argCount < 3)
    {
        printf("Could not add variable: invalid amount of arguments\n");
        return;
    }
    char *valueStr = join(pCmdLine->arguments, 2, pCmdLine->argCount, " ");
    addMyShellVar(pCmdLine->arguments[1], valueStr, varList);
    free(valueStr);
}

void sendSignal(char *signame, pid_t pid)
{
    int killRes = 0;
    if (strcmp(signame, "kill") == 0)
    {
        killRes = kill(pid, SIGINT);
    }
    else if (strcmp(signame, "suspend") == 0)
    {
        killRes = kill(pid, SIGTSTP);
    }
    else if (strcmp(signame, "wake") == 0)
    {
        killRes = kill(pid, SIGCONT);
    }
    if (killRes < 0)
    {
        perror(signame);
    }
    else
    {
        puts("Signal sent successfully");
    }
}

void addMyShellVar(char *name, char *value, myShellVar **varList)
{
    myShellVar *replace = NULL;
    if (varList == NULL)
    {
        fprintf(stderr, "Faild to set var: Invalid var list.");
        return;
    }
    else if ((replace = getVar(name, varList)) != NULL)
    {
        myShellVarSetVal(replace, value);
    }
    else
    {
        *varList = myShellVarCREATE(name, value, *varList);
    }
}

void freeVarList(myShellVar **varList)
{
    if (varList != NULL)
    {
        myShellVarDESTROY(*varList);
        free(varList);
    }
}

myShellVar *getVar(const char *name, myShellVar **varList)
{
    myShellVar *cur = *varList;
    while (cur != NULL)
    {
        if (strcmp(cur->name, name) == 0)
        {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int replaceMyShellVars(cmdLine *pCmdLine, myShellVar **varList)
{
    myShellVar *var = NULL;
    char *arg = NULL;
    for (int i = 0; i < pCmdLine->argCount; i++)
    {
        arg = pCmdLine->arguments[i];
        if (strncmp(arg, "$", 1) == 0)
        {
            if ((var = getVar(arg + 1, varList)) != NULL)
            {
                replaceCmdArg(pCmdLine, i, var->value);
            }
            else
            {
                fprintf(stderr, "Failed to raplce var: variable that does not exist.");
                return -1;
            }
        }
    }
    return 0;
}

void printVarList(myShellVar **varList)
{
    if (varList == NULL)
    {
        fprintf(stderr, "Failed printing: Invalid var list.");
        return;
    }
    printf("    SHELL VARS\n");
    printf("NAME          VALUE\n");
    myShellVar *cur = *varList;
    while (cur != NULL)
    {
        printf("%s             %s\n", cur->name, cur->value);
        cur = cur->next;
    }
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    if (process_list == NULL)
    {
        printf("Invalid process list");
        return;
    }
    *process_list = processCREATE(cmd, pid, RUNNING, *process_list);
}

void printProcessList(process **process_list)
{
    process *proc = *process_list;
    char *command = NULL, *status = NULL;
    int index = 1;

    updateProcessList(process_list);

    puts("|INDEX   PID     STATUS            COMMAND: ARGUMETS");
    while (proc != NULL)
    {
        command = proc->cmd->arguments[0];
        status = getStatusName(proc->status);
        printf("|%d)      %d  |  %s     |     %s: ", index, proc->pid, status, command);
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
    if (process_list != NULL)
    {
        processDESTROY(*process_list);
        free(process_list);
    }
}

void updateProcessList(process **process_list)
{
    int wstatus = 0;
    process *proc = *process_list;
    int waitResult = 0;
    while (proc)
    {
        wstatus = 0;
        waitResult = waitpid(proc->pid, &wstatus, WNOHANG | WUNTRACED | WCONTINUED);
        if (waitResult > 0)
        {
            chooseUpdateAction(wstatus, proc);
        }
        else if (waitResult < 0)
        {
            perror("procs");
        }
        proc = proc->next;
    }
}

void chooseUpdateAction(int wstatus, process *proc)
{
    if (WIFSTOPPED(wstatus))
    {
        puts("suspending");
        updateProcessStatus(proc, proc->pid, SUSPENDED);
    }
    else if (WIFCONTINUED(wstatus))
    {
        puts("waking");
        updateProcessStatus(proc, proc->pid, RUNNING);
    }
    else if (WIFSIGNALED(wstatus) || WIFEXITED(wstatus))
    {
        puts("killing");
        updateProcessStatus(proc, proc->pid, TERMINATED);
    }
}

void updateProcessStatus(process *process_list, pid_t pid, int status)
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
    process *proc = calloc(1, sizeof(process));
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

void debugger(char *command, pid_t pid)
{
    if (_debug)
    {
        fprintf(stderr, "Currnet command: %s\nProccess ID: %i\n", command, pid);
    }
}

myShellVar *myShellVarCREATE(char *name, char *value, myShellVar *next)
{
    myShellVar *var = calloc(1, sizeof(myShellVar));
    char *copyName = strClone(name);
    char *copyValue = strClone(value);
    myShellVarCtor(var, copyName, copyValue, next);
    return var;
}

void myShellVarCtor(myShellVar *var, char *name, char *value, myShellVar *next)
{
    var->name = name;
    var->value = value;
    var->next = next;
}

void myShellVarDESTROY(myShellVar *var)
{
    if (var != NULL)
    {
        myShellVarDESTROY(var->next);
        myShellVarDtor(var);
        free(var);
    }
}

void myShellVarDtor(myShellVar *var)
{
    free(var->name);
    free(var->value);
}

void myShellVarSetVal(myShellVar *var, char *value)
{
    var->value = strClone(value);
}

void freeLine(cmdLine *pCmdLine)
{
    freeCmdLines(pCmdLine);
}

char *strClone(char *str)
{
    return strcpy(calloc(strlen(str) + 1, sizeof(char)), str);
}

char *join(char *const *strArr, int start, int end, const char *seperator)
{
    char *acc = strClone(strArr[start]);
    for (int i = start + 1; i < end; i++)
    {
        acc = realloc(acc, strlen(acc) + strlen(seperator) + strlen(strArr[i]) + 1);
        strncat(acc, seperator, strlen(seperator));
        strncat(acc, strArr[i], strlen(strArr[i]));
    }
    return acc;
}

char *concat(char *str1, char *str2)
{
    char *acc = strClone(str1);
    acc = realloc(acc, strlen(str1) + strlen(str2) + 1);
    return strncat(acc, str2, strlen(str2));
}