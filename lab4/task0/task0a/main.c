#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1

extern int system_call();

int main(int argc, char *argv[], char *envp[])
{
  system_call(SYS_WRITE, STDOUT, "Hello World\n", 12);
  return 0;
}
