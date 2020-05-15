#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>

int main(int argc, char *argv[])
{
    int pipefd[2];
    pid_t c1pid = 0;
    char *lsCmd[3], *tailCmd[4];
    lsCmd[0] = "ls";
    lsCmd[1] = "-l";
    lsCmd[2] = NULL;
    tailCmd[0] = "tail";
    tailCmd[1] = "-n";
    tailCmd[2] = "2";
    tailCmd[3] = NULL;
    pid_t c2pid = 0;
    int c1res = 0, c2res = 0;
    bool debug = 0;

    if (strncmp(argv[1], "-d", 2) == 0)
    {
        debug = 1;
    }

    if (pipe(pipefd) == -1)
    {
        perror("Could not open pipe");
        exit(EXIT_FAILURE);
    }

    if (debug)
        fprintf(stderr, "(parent_process>forking...)\n");
    if ((c1pid = fork()) == -1)
    {
        perror("Could not fork");
        exit(EXIT_FAILURE);
    }

    if (debug && c1pid > 0)
        fprintf(stderr, "(parent_process>created process with id: %d)\n", c1pid);

    if (c1pid == 0)
    {
        if (debug)
            fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        close(1);
        dup(pipefd[1]);
        close(pipefd[1]);
        if (debug)
            fprintf(stderr, "(child1>going to execute cmd: ...)\n");
        execvp(lsCmd[0], lsCmd);
        perror("Could not execute command");
        exit(EXIT_SUCCESS);
    }

    if (debug)
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    close(pipefd[1]);

    if (debug)
        fprintf(stderr, "(parent_process>forking...)\n");
    if ((c2pid = fork()) == -1)
    {
        perror("Could not fork");
        exit(EXIT_FAILURE);
    }
    if (debug && c2pid > 0)
        fprintf(stderr, "(parent_process>created process with id: %d)\n", c2pid);

    if (c2pid == 0)
    {
        if (debug)
            fprintf(stderr, "(child2>redirecting stdout to the read end of the pipe...)\n");
        close(0);
        dup(pipefd[0]);
        close(pipefd[0]);
        if (debug)
            fprintf(stderr, "(child2>going to execute cmd: ...)\n");
        execvp(tailCmd[0], tailCmd);
        perror("Could not execute command");
        exit(EXIT_SUCCESS);
    }

    if (debug)
        fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
    close(pipefd[0]);

    if (debug)
        fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    c1res = waitpid(c1pid, NULL, 0);
    c2res = waitpid(c2pid, NULL, 0);

    if (debug)
        fprintf(stderr, "(parent_process>exiting...)\n");
    if (c1res < 0 || c2res < 0)
    {
        perror("Child process error");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}