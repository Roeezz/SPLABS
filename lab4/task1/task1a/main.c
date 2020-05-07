#include "util.h"

#define SYS_WRITE 4

#define SYS_LSEEK 19

#define SYS_READ 3

#define SYS_OPEN 5
#define O_RDONLY 0 /*read only*/
#define O_WRONLY 1 /*write only*/
#define O_RDRW 2

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
int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3, int debug);

void encoder(int debug, int input, int output);
char uppercase(char original, int output);

int main(int argc, char const *argv[])
{
  int input = STDIN, output = STDOUT;
  int debug = 0;

  int i;
  for (i = 1; i < argc; i++)
  {
    if (strncmp(argv[i], "-D", 2) == 0)
    {
      debug = 1;
    }
    else
    {
      char *err_msg = "Illegal program argument";
      system_call(output, STDERR, err_msg, strlen(err_msg));
      return 1;
    }
  }
  encoder(debug, input, output);
  return 0;
}

void encoder(int debug, int input, int output)
{
  char original = -1, encoded = -1, call_ret = 0;

  call_ret = (sys_call_handler(SYS_READ, input, (unsigned int)&original, 1, debug));
  while (call_ret > 0)
  {
    if (original == '\n')
    {
      sys_call_handler(SYS_WRITE, output, (unsigned int)"\n", 1, debug);
    }
    else
    {
      encoded = uppercase(original, output);
      sys_call_handler(SYS_WRITE, output, (unsigned int)&encoded, 1, debug);
    }
    call_ret = (sys_call_handler(SYS_READ, input, (unsigned int)&original, 1, debug));
  }
}

char uppercase(char original, int output)
{
  if (LOW_LETTER <= original && original <= HIGH_LETTER)
  {
    original = original - MODIF;
  }

  return original;
}

int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3, int debug)
{
  int result = system_call(call, arg1, arg2, arg3);
  if (debug == 1)
  {
    char *str = itoa(call);
    system_call(SYS_WRITE, STDERR, str, strlen(str));
    system_call(SYS_WRITE, STDERR, " ", 1);

    str = itoa(result);
    system_call(SYS_WRITE, STDERR, str, strlen(str));
    system_call(SYS_WRITE, STDERR, "\n", 1);
  }
  return result;
}