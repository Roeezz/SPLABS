#include "util.h"

#define SYS_WRITE 4

#define SYS_LSEEK 19
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define SYS_READ 3

#define SYS_OPEN 5
#define O_RDONLY 0 /*read only*/
#define O_WRONLY 1 /*write only*/
#define O_RDRW 2
#define O_CREATE 64
#define O_TURNCATE 512
#define O_APPEND 1024

#define SYS_CLOSE 6

#define SYS_EXIT 1
#define SUCSSEFUL_EXIT_CODE 0
#define UNSUCSSEFUL_EXIT_CODE 0x55

/*standard I/O */
#define STDOUT 1
#define STDIN 0
#define STDERR 2

#define MODIF 32
#define LOW_LETTER 97
#define HIGH_LETTER 122

#define EOF (-1)

extern int system_call();

int main(int argc, char *argv[], char *envp[])
{
	char *file_name = argv[0];
	char *x_name = argv[1];
	int fd = 0, file_size = 0;
	
	fd = system_call(SYS_OPEN, file_name, O_RDRW, 0777);
	file_size = system_call(SYS_LSEEK, fd, 0, SEEK_END);
	system_call(SYS_LSEEK, fd, 0, SEEK_SET);

	int i;
	for (i = 0; i < file_size; i++)
	{
		
	}

	return 0;
}
