#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "process.h"
#include "pagedir.h"
#include "threads/init.h"

static void syscall_handler (struct intr_frame *);
static void is_valid_ptr (const void *vaddr);
static struct lock fs_lock;
static void* stack_pop(void ** esp, int bytes);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{ /*P2*/
 is_valid_ptr(f->esp);
 //int syscall_num = * (int *) f->esp;
 void *stack = f->esp;
 struct thread *t = thread_current();
 int exit_status;
 tid_t wait_tid;
 char * cmd_line;
 char * file_name;
 
 int syscall_num = *(int *) stack_pop(&stack,sizeof(int));
 switch(syscall_num)
 {
    case SYS_HALT:
      thread_exit();
      shutdown_power_off();
      break;

    case SYS_EXIT:
      /*set exit status */ 
      exit_status = *(int *) stack_pop(&stack,sizeof(int));
      t->our_child_self.exit_status = exit_status;

      /* is our parent waiting on us? */
      if (t->parent->child_waiting_on == t->tid) 
      {
        /* preemtion might occur here maybe it breaks shit*/
        sema_up(&t->parent->child_wait_sema);
      }
      
      /*lastly call process exit*/
      printf ("%s: exit(%d)\n", t->name, exit_status);
      thread_exit();
      break;

    case SYS_EXEC:
      cmd_line = * (char **) stack_pop(&stack, sizeof(char *));
      process_execute(cmd_line);
      break;
    case SYS_WAIT:
      wait_tid = *(tid_t *) stack_pop(&stack, sizeof(tid_t));
      f->eax = process_wait(wait_tid);
      break;

    case SYS_CREATE:
      lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack,sizeof(char*));
      int file_size =  *(int*) stack_pop(&stack,sizeof(int));
      filesys_create(file_name, file_size); 
      lock_release(&fs_lock);
      break;

    case SYS_REMOVE:
     /* lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      if(filesys_remove(file_name)){
        f->eax = 1;
        lock_release(&fs_lock);
        break;
      }
      else{
        thread_exit();
      } */
      printf("***REMOVING***\n");
      break;

    case SYS_OPEN:
      /*lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      f->eax = filesys_open(file_name);
      lock_release(&fs_lock);*/
      printf("***OPENING***\n");
      break;

    case SYS_FILESIZE:
      printf("***FILESIZE***\n");
      break;

    case SYS_READ:
      printf("***READING***\n");
      break;

    case SYS_WRITE:
      if(0);
      int fd = *(int *)stack_pop(&stack,sizeof(int));
      void * buffer = *(void **)stack_pop(&stack,sizeof(void *));
      unsigned size = *(unsigned *)stack_pop(&stack,sizeof(unsigned *));
      ASSERT(is_user_vaddr(buffer));
      /* if(!is_user_vaddr(buffer)){
        ASSERT(0)
        thread_exit();
      } */
      void *pg_ptr = pagedir_get_page(t->pagedir, buffer);
      ASSERT(pg_ptr != NULL);
      /*if(pg_ptr == NULL){
        thread_exit();
      }*/
      if(fd == 1){
        putbuf((char *) pg_ptr, size);
      }
      f->eax = (uint32_t)size;
      break;

    case SYS_SEEK:
     printf("***SEEKING***\n");
      break;

    case SYS_TELL:
    printf("***TELLING***\n");
      break;

    case SYS_CLOSE:
    printf("***CLOSING***\n");
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


static void is_valid_ptr(const void *vaddr)
{
  if(!is_user_vaddr(vaddr))
  {
    ASSERT(0);
  }
  
}

/*P2*/
static void* stack_pop(void **esp, int bytes)
{
  void *data_popped= *esp;
  *esp =  *esp + bytes;
  return data_popped;
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
       
//      filesys_create(*(char**)stack_pop(&stack,sizeof(char*)), *(int*) stack_pop(&stack,sizeof(int)));
//      printf("filename:%s  \n",*(char**)stack_pop(&stack,sizeof(char*)));
//      printf("size is %d\n",*(int*) stack_pop(&stack,sizeof(int)));

     //hex_dump(stack,stack,40,1);