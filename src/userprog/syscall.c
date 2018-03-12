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

/*P2*/
/* Magic number borrowed from ryantimwilson */
#define VA_BOTTOM (void*)0x08048000

//static struct child * get_child(int tid);
static void syscall_handler (struct intr_frame *);
static void is_valid_ptr (const void *vaddr);
static struct lock fs_lock;
static void* stack_pop(void ** esp, int bytes);
static void * va_to_pa(void * va);
static int init_thread_file_struct(struct file * f);
static struct file *get_open_file (int fd);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{ /*P2*/
 struct thread *t = thread_current();
// printf("our pointer = %p\n our tid = %d\n" ,f->esp, t->tid);  
 is_valid_ptr(f->esp);
 va_to_pa(f->esp);
 //int syscall_num = * (int *) f->esp;
 void *stack = f->esp;
 int exit_status;
 tid_t wait_tid;
 char * cmd_line;
 char * file_name;
 
 int syscall_num = *(int *) stack_pop(&stack,sizeof(int));
 switch(syscall_num)
 {
    case SYS_HALT:
      // thread_exit();
      shutdown_power_off();
      break;

    case SYS_EXIT:
      /*set exit status */
      exit_status = *(int *) stack_pop(&stack,sizeof(int));
      
      sys_exit(exit_status);


      break;

    case SYS_EXEC:
      cmd_line = * (char **) stack_pop(&stack, sizeof(char *));
      is_valid_ptr(cmd_line); 
      int tid = process_execute((char*)va_to_pa((void*)cmd_line));
      struct child * child_to_exec;
      child_to_exec = get_child_process(tid);

      ASSERT(child_to_exec != NULL);  

      while(child_to_exec->load_success == NOT_LOADED)
      {
        thread_yield();
      }
      if(child_to_exec->load_success == LOAD_FAILED)
      {
        tid = TID_ERROR; 

      }


      f->eax = tid;
      break;

      
    case SYS_WAIT:
      wait_tid = *(tid_t *) stack_pop(&stack, sizeof(tid_t));
      f->eax = process_wait(wait_tid);
      break;

    case SYS_CREATE:
      lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack,sizeof(char*));
      is_valid_ptr(file_name); 
      int file_size =  *(int*) stack_pop(&stack,sizeof(int));
      f->eax = filesys_create((char*)va_to_pa((void*)file_name), file_size); 
      lock_release(&fs_lock);
      break;

    case SYS_REMOVE:
    {
      lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      is_valid_ptr(file_name);
      f->eax = filesys_remove(va_to_pa(file_name));
      lock_release(&fs_lock);
      break;
    }
    case SYS_OPEN:
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      is_valid_ptr(file_name);

      lock_acquire(&fs_lock);
      struct file *file_struct = filesys_open((char *)va_to_pa((void *)file_name));
      lock_release(&fs_lock);
      
      f->eax = init_thread_file_struct(file_struct);
      break;

    case SYS_FILESIZE:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int*));
      lock_acquire(&fs_lock);
      struct file *file = get_open_file(fd);
      off_t file_size = file_length(file);
      f->eax = file_size;
      lock_release(&fs_lock);

      break;
    }
    case SYS_READ:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int*));
      void *buffer = stack_pop(&stack, sizeof(void*));
      unsigned size = *(unsigned *)stack_pop(&stack, sizeof(unsigned *));

      if(!is_user_vaddr(vaddr) || vaddr < VA_BOTTOM || vaddr == NULL)
      {
        f->eax = -1;
        break;
      }

      buffer = pagedir_get_page(thread_current()->pagedir, buffer);
      if(buffer == NULL)
      {
        f->eax = -1;
        break;
      }

      struct file *file = get_open_file(fd);

      lock_acquire(&fs_lock);
      off_t bytes_read = file_read (file, buffer, size);
      lock_release(&fs_lock);

      f->eax = bytes_read;

      break;
    }
    case SYS_WRITE:
    {
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
    }
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


/*P2*/
/* TODO RENAME maybe something like ... exit_if_bad_ptr*/
static void is_valid_ptr(const void *vaddr)
{
  if(!is_user_vaddr(vaddr) || vaddr < VA_BOTTOM || vaddr == NULL)
  {
    sys_exit(-1);
  }
  
}

/*P2*/
static void* stack_pop(void **esp, int bytes)
{
  void *data_popped= *esp;
  is_valid_ptr(data_popped);
  *esp =  *esp + bytes;
  return data_popped;
}

/*P2*/
void sys_exit(int exit_status)
{
  struct thread *t = thread_current();
  t->our_child_self->exit_status = exit_status;

  /* is our parent waiting on us? */
  //printf("t->parent->child_waiting_on = %d ;  t->tid = %d\n", t->parent->child_waiting_on, t->tid);
  printf ("%s: exit(%d)\n", t->name, exit_status);
//  printf ("parent waiting on %d\n", t->parent->child_waiting_on);
  if (t->parent->child_waiting_on == t->tid) 
  {
    /* preemtion might occur here maybe it breaks shit*/
    sema_up(&t->parent->child_wait_sema);
  }
  /*lastly call process exit*/
  thread_exit();
  NOT_REACHED();
}

/*P2*/
static void * va_to_pa(void * va)
{
  void * pa = pagedir_get_page(thread_current()->pagedir, va);
  if(pa == NULL)
  {
    sys_exit(-1);
  }
  return pa;
}


/*returns a file descriptor for the given file struct*/
static int init_thread_file_struct(struct file * file_struct)
{
  struct thread *t = thread_current();
  struct an_open_file threads_file;

  if (file_struct == NULL) return -1; /* if the file doesnt exit return bad fd*/

  threads_file.file = file_struct;
  threads_file.file_descriptor = t->next_file_descriptor++; /* get the next fd and increment it after */

  list_push_back(&t->open_files,&threads_file.elem);

  return threads_file.file_descriptor;
}


/*P2*/
/* returns a file struct for the given file descriptor*/
/* if fd doesnt match an open file, returns NULL*/
static struct file *get_open_file (int fd) 
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for(e = list_begin(&t->open_files); e != list_end(&t->open_files); e=list_next(e))
  {
    struct an_open_file *an_open_f = list_entry(e, struct an_open_file, elem );
    if(fd == an_open_f->file_descriptor )
    {
      return an_open_f->file;
    }
  }
  return NULL;
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
/*
static struct child * get_child(int tid)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for(e = list_begin(&t->children); e != list_end(&t->children); e = list_next(e))
  {
    struct child *cp = list_entry(e, struct child, child_elem);
    if(tid == cp->child_tid)
    {
      return cp;
    }
  }
  return NULL;
}
*/