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
  //write(1, filename, 8);
  int fd = open("asd.txt");
  printf("our file's size is %d\n",filesize(fd));

  fd = open("echo");
  printf("our file's size is %d\n",filesize(fd));

  fd = open("cp");
  printf("our file's size is %d\n",filesize(fd));
  halt ();
  /* not reached */
}
