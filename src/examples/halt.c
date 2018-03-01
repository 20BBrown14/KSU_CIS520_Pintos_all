/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  //P2//
  printf("*****FUCKING PRINTS*******");
  halt ();
  /* not reached */
}
