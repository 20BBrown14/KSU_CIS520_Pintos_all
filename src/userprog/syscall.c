#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{ /* Added for project 2 */
 //void *esp = f->esp;
 int32_t * syscall_num = (int32_t *)f->esp;
 //printf("System Call Number: %d\n", *syscall_num);
 //void *arg1 = *esp+4;
 //void *arg2 = *esp+8;
 
 switch(*syscall_num)
 {
    case SYS_HALT:
      printf ("%s: exit(%d)\n", "thread", 2);
      shutdown_power_off();
      
      break;
    case SYS_EXIT:
      break;
    case SYS_EXEC:
      break;
    case SYS_WAIT:
      break;
    case SYS_CREATE:
      break;
    case SYS_REMOVE:
      break;
    case SYS_OPEN:
      break;
    case SYS_FILESIZE:
      break;
    case SYS_READ:
      break;
    case SYS_WRITE:
      break;
    case SYS_SEEK:
      break;
    case SYS_TELL:
      break;
    case SYS_CLOSE:
      break;
    default:
      printf("***SYSTEM CALL ERROR***\n");
      break;

 }

 /* Removed for Project 2
  //printf ("system call!\n");
  //thread_exit ();
  */
}


// added for project 2
//System call numbers for reference
//left numbers are the enum numbers for the sys_call, right side of numbers is the number of arguments for that call
//  0  SYS_HALT,                   /* Halt the operating system. */         0
//  1  SYS_EXIT,                   /* Terminate this process. */            1
//  2  SYS_EXEC,                   /* Start another process. */             1
//  3  SYS_WAIT,                   /* Wait for a child process to die. */   1
//  4  SYS_CREATE,                 /* Create a file. */                     2
//  5  SYS_REMOVE,                 /* Delete a file. */                     1
//  6  SYS_OPEN,                   /* Open a file. */                       1
//  7  SYS_FILESIZE,               /* Obtain a file's size. */              1
//  8  SYS_READ,                   /* Read from a file. */                  3
//  9  SYS_WRITE,                  /* Write to a file. */                   3
//  10 SYS_SEEK,                   /* Change position in a file. */         2
//  11 SYS_TELL,                   /* Report current position in a file. */ 1
//  12 SYS_CLOSE,                  /* Close a file. */                      1