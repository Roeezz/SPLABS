#include "util.h"

#define SYS_GETDENTS 141

#define SYS_EXIT 1
#define E_ERR 0x55
#define E_NORMAL 0

#define SYS_WRITE 4

#define SYS_LSEEK 19

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

#define P_USRPERM 0700

#define EOF (-1)
#define NULL (void *)0
#define BUFF_SIZE 8192

enum
{
	DT_UNKNOWN = 0,
#define DT_UNKNOWN DT_UNKNOWN
	DT_FIFO = 1,
#define DT_FIFO DT_FIFO
	DT_CHR = 2,
#define DT_CHR DT_CHR
	DT_DIR = 4,
#define DT_DIR DT_DIR
	DT_BLK = 6,
#define DT_BLK DT_BLK
	DT_REG = 8,
#define DT_REG DT_REG
	DT_LNK = 10,
#define DT_LNK DT_LNK
	DT_SOCK = 12,
#define DT_SOCK DT_SOCK
	DT_WHT = 14
#define DT_WHT DT_WHT
};

typedef struct ent
{
	int innode;
	int offset;
	short len;
	char name[];
} ent;

extern int system_call();
int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3);
void debugger(int call, int result);

void whitespace();
void newline();

int flame();
void printAllFiles(char buffer[], int bytesRead);

int _debug = 0;
char *_prefix = NULL;

int main(int argc, char const *argv[])
{
	int i;
	for (i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "-D", 2) == 0)
		{
			_debug = 1;
		}
		else if (strncmp(argv[i], "-p", 2) == 0)
		{
			/*TODO*/
		}
		else if (strncmp(argv[i], "-a", 2) == 0)
		{
			/*TODO*/
		}
		else
		{
			char *err_msg = "Illegal program argument";
			system_call(SYS_WRITE, STDERR, (unsigned int)err_msg, strlen(err_msg));
			return E_ERR;
		}
	}
	return flame();
}

int flame()
{
	int curDir = 0, bytesRead = 0;
	char buffer[BUFF_SIZE];

	curDir = sys_call_handler(SYS_OPEN, (unsigned int)".", 0, 0);
	if (curDir > 0)
	{
		bytesRead = sys_call_handler(SYS_GETDENTS, curDir, (unsigned int)buffer, BUFF_SIZE);
		sys_call_handler(SYS_CLOSE, curDir, 0, 0);
		printAllFiles(buffer, bytesRead);
	}
	else
	{
		return E_ERR;
	}
	return E_NORMAL;
}

void printAllFiles(char buffer[], int bytesRead)
{
	ent *files = NULL;
	files = (ent *)buffer;
	int i;
	for (i = 0; i < bytesRead; i += files->len)
	{
		files = (ent *)(buffer + i);
		sys_call_handler(SYS_WRITE, STDOUT, (unsigned int)files->name, strlen(files[0].name));
		newline(STDOUT);
		if(_debug) {
			system_call(SYS_WRITE, STDOUT, itoa(files->len), strlen(itoa(files->len)));
			newline(STDOUT);
		}
	}
}

void whitespace(int output)
{
	sys_call_handler(SYS_WRITE, output, (unsigned int)" ", 1);
}

void newline(int output)
{
	sys_call_handler(SYS_WRITE, output, (unsigned int)"\n", 1);
}

int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	int result = system_call(call, arg1, arg2, arg3);
	if (_debug == 1)
	{
		debugger(call, result);
	}
	return result;
}

void debugger(int call, int result)
{
	char *str = itoa(call);
	system_call(SYS_WRITE, STDERR, str, strlen(str));
	system_call(SYS_WRITE, STDERR, " ", 1);

	str = itoa(result);
	system_call(SYS_WRITE, STDERR, str, strlen(str));
	system_call(SYS_WRITE, STDERR, "\n", 1);
}
