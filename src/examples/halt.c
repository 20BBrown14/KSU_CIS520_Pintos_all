/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  /*P2*/
  /* using halt to test file create for now*/
  /* TODO REMOVE MAKE COMMANDS */
  char filename [7] = "asd.txt";
  create(filename, 64);
  halt ();
  /* not reached */
}
