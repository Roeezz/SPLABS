#include "util.h"

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

#define USER_PERMITTED 0700

#define DEBUG 1

#define EOF (-1)

extern int system_call();
int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3);
void debugger(int call, int result);

int _debug = 0;
const char *_input = "stdin", *_output = "stdout";

void encoder(int input, int output);
char uppercase(char original);

int main(int argc, char const *argv[])
{
  int input = STDIN, output = STDOUT;

  int i;
  for (i = 1; i < argc; i++)
  {
    if (strncmp(argv[i], "-D", 2) == 0)
    {
      _debug = 1;
    }
    else if (strncmp(argv[i], "-o", 2) == 0)
    {
      output = sys_call_handler(SYS_OPEN, (unsigned int)(argv[i] + 2), O_WRONLY | O_CREATE | O_TURNCATE, USER_PERMITTED);
      _output = argv[i] + 2;
    }
    else if (strncmp(argv[i], "-i", 2) == 0)
    {
      input = sys_call_handler(SYS_OPEN, (unsigned int)(argv[i] + 2), O_RDONLY, USER_PERMITTED);
      _input = argv[i] + 2;
      if (input < 0)
      {
        char *err_msg = "Could not open file";
        system_call(SYS_WRITE, STDERR, err_msg, strlen(err_msg));
        return 1;
      }
    }
    else
    {
      char *err_msg = "Illegal program argument";
      system_call(SYS_WRITE, STDERR, err_msg, strlen(err_msg));
      return 1;
    }
  }
  encoder(input, output);
  sys_call_handler(SYS_CLOSE, output, 0, 0);
  return 0;
}

void encoder(int input, int output)
{
  char original = -1, encoded = -1, call_ret = 0;

  call_ret = (sys_call_handler(SYS_READ, input, (unsigned int)&original, 1));
  while (call_ret > 0)
  {
    if (original == '\n')
    {
      sys_call_handler(SYS_WRITE, output, (unsigned int)"\n", 1);
    }
    else
    {
      encoded = uppercase(original);
      sys_call_handler(SYS_WRITE, output, (unsigned int)&encoded, 1);
    }
    call_ret = (sys_call_handler(SYS_READ, input, (unsigned int)&original, 1));
  }
}

char uppercase(char original)
{
  if (LOW_LETTER <= original && original <= HIGH_LETTER)
  {
    original = original - MODIF;
  }

  return original;
}

int sys_call_handler(int call, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
  int result = system_call(call, arg1, arg2, arg3);
  if (_debug & DEBUG)
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
  system_call(SYS_WRITE, STDERR, " ", 1);

  system_call(SYS_WRITE, STDERR, _input, strlen(_input));
  system_call(SYS_WRITE, STDERR, " ", 1);

  system_call(SYS_WRITE, STDERR, _output, strlen(_output));
  system_call(SYS_WRITE, STDERR, "\n", 1);
}
