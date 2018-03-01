#include <syscall.h>
#include <stdio.h>

int main (int, char *[]);
void _start (int argc, char *argv[]);

void
_start (int argc, char *argv[]) 
{
  /*P2*/
  ASSERT(false);
  //printf("argc: %d, argv[0] = %s, argv[1] = %s\n", argc, argv[0], argv[1]);
  vprintf(1,"vtesting");
  exit (main (argc, argv));
}
