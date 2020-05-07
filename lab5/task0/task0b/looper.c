#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void sigHandler(int signum);

int main(int argc, char **argv)
{

	printf("Starting the program\n");
	signal(SIGTSTP, sigHandler);
	signal(SIGCONT, sigHandler);
	signal(SIGINT, sigHandler);
	while (1)
	{
		sleep(2);
	}
	
	return 0;
}

void sigHandler(int signum)
{
	char *signame = strsignal(signum);
	printf("Recieved %s\n", signame);
	signal(signum, SIG_DFL);
	if (signum && SIGTSTP)
	{
		signal(SIGCONT, sigHandler);
	}
	else if (signum && SIGCONT)
	{
		signal(SIGTSTP, sigHandler);
	}
	raise(signum);
}