#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "process.h"
#include "pagedir.h"
#include "threads/init.h"

/*P2*/
#define VA_BOTTOM (void*)0x08048000

static void syscall_handler (struct intr_frame *);
static void is_valid_ptr (const void *vaddr);
static struct lock fs_lock;
static void* stack_pop(void ** esp, int bytes);
static void * user_to_kernel(void * va);
static int init_thread_file_struct(struct file * f);
static struct file *get_open_file (int fd);

static void rm_file_from_open_files(int fd);
static void is_valid_ptr_range(const void *vaddr);
static void validate_data_ptr(void *data);

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

 is_valid_ptr(f->esp);
  
 user_to_kernel(f->esp); //Just using this to check validity, not do a conversion.
 void *stack = f->esp;
 int exit_status;
 tid_t wait_tid;
 char * cmd_line;
 char * file_name;
 
 
 int syscall_num = *(int *) stack_pop(&stack,sizeof(int));
 switch(syscall_num)
 {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      /*set exit status */
      exit_status = *(int *) stack_pop(&stack,sizeof(int));

      
      sys_exit(exit_status);


      break;

    case SYS_EXEC:
    {
      cmd_line = * (char **) stack_pop(&stack, sizeof(char *));
      
      is_valid_ptr(cmd_line);
      int tid = process_execute((char*)user_to_kernel((void*)cmd_line));
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
    }

      
    case SYS_WAIT:
      wait_tid = *(tid_t *) stack_pop(&stack, sizeof(tid_t));
      f->eax = process_wait(wait_tid);
      break;

    case SYS_CREATE:
      lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack,sizeof(char*));
      is_valid_ptr(file_name); 
      int file_size =  *(int*) stack_pop(&stack,sizeof(int));
      f->eax = filesys_create((char*)user_to_kernel((void*)file_name), file_size); 
      lock_release(&fs_lock);
      break;

    case SYS_REMOVE:
    {
      lock_acquire(&fs_lock);
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      is_valid_ptr(file_name);
      f->eax = filesys_remove(user_to_kernel(file_name));
      lock_release(&fs_lock);
      break;
    }
    case SYS_OPEN:
      file_name = *(char**)stack_pop(&stack, sizeof(char*));
      is_valid_ptr(file_name);

      lock_acquire(&fs_lock);
      struct file *file_struct = filesys_open((char *)user_to_kernel((void *)file_name));
      lock_release(&fs_lock);
      
      f->eax = init_thread_file_struct(file_struct);
      break;

    case SYS_FILESIZE:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int*));
      lock_acquire(&fs_lock);
      struct file *file = get_open_file(fd); 
      if(file == NULL){
        lock_release(&fs_lock);
        sys_exit(-1);
        break;
      }
      off_t file_size = file_length(file);
      f->eax = file_size;
      lock_release(&fs_lock);

      break;
    }
    case SYS_READ:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int*));
      void *buffer =  *(void **)stack_pop(&stack, sizeof(void*));
      unsigned size = *(unsigned *)stack_pop(&stack, sizeof(unsigned *));
      if(!is_user_vaddr(buffer) || buffer < VA_BOTTOM || buffer == NULL)
      {
        sys_exit(-1);
        break;
      }
      void * pg_ptr = pagedir_get_page(thread_current()->pagedir, (const void *)buffer);
      if(pg_ptr == NULL)
      {
        sys_exit(-1);
        break;
      }
      lock_acquire(&fs_lock);
      struct file *file = get_open_file(fd);
      if(file == NULL){
        lock_release(&fs_lock);
        sys_exit(-1);
        break;
      }
      f->eax = (uint32_t)file_read (file, pg_ptr, size);
      lock_release(&fs_lock);
      break;
    }
    case SYS_WRITE:
    {
      int fd = *(int *)stack_pop(&stack,sizeof(int));
      void * buffer = *(void **)stack_pop(&stack,sizeof(void *));
      unsigned size = *(unsigned *)stack_pop(&stack,sizeof(unsigned *));
      ASSERT(is_user_vaddr(buffer));

      void *pg_ptr = pagedir_get_page(t->pagedir, buffer);
      if(pg_ptr == NULL){
        sys_exit(-1);
        break;
      }
      if(fd == 1){
        putbuf((char *) pg_ptr, size);
        f->eax = (uint32_t)size;
        break;
      }
      lock_acquire(&fs_lock);
      struct file *the_file = get_open_file(fd);
      if(the_file == NULL){
        lock_release(&fs_lock);
        sys_exit(-1);
      }
      f->eax = file_write(the_file, pg_ptr, size);
      lock_release(&fs_lock);
      break;
    }
    case SYS_SEEK:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int));
      unsigned position = *(unsigned *)stack_pop(&stack, sizeof(unsigned));
      lock_acquire(&fs_lock);
      struct file *the_file = get_open_file(fd);
      if(the_file == NULL){
        lock_release(&fs_lock);
        sys_exit(-1);
      }
      file_seek(the_file, position);
      lock_release(&fs_lock);
      break;
    }

    case SYS_TELL:
    {
      int fd = *(int *)stack_pop(&stack, sizeof(int));
      lock_acquire(&fs_lock);
      struct file *the_file = get_open_file(fd);
      if(the_file == NULL){
        lock_release(&fs_lock);
        sys_exit(-1);
      }
      f->eax = file_tell(the_file);
      lock_release(&fs_lock);
      break;
    }

    case SYS_CLOSE:
    {
      int fd = *(int *)stack_pop(&stack,sizeof(int));
      lock_acquire(&fs_lock);
      struct file *file = get_open_file(fd);
      if (file == NULL)
      {
        lock_release(&fs_lock);
        sys_exit(-1);
        break;
      }
      file_close(file);
      rm_file_from_open_files(fd); 
      lock_release(&fs_lock);
      break;
    }

    default:
      printf("***SYSTEM CALL ERROR***\n");
      break;

 }

 /* Removed for Project 2*/
  //printf ("system call!\n");
  //thread_exit ();
}


/*P2*/
static void is_valid_ptr(const void *vaddr)
{
    void * local_vaddr = vaddr;
    if(!is_user_vaddr(vaddr) || vaddr < VA_BOTTOM || vaddr == NULL)
    {
      sys_exit(-1);
    }

    /* when checking for arguments that go past the frame bountry we found strange behavior */
    /* this is a terrible solution but works to catch the issue */
    for(int i = 0; i < 4; i++)
    {
      user_to_kernel(local_vaddr);
      local_vaddr++;
    }
}


static void validate_data_ptr(void *data)
{
  if(!is_user_vaddr(data) || data < VA_BOTTOM || data == NULL)
    {
      sys_exit(-1);
    }

    user_to_kernel(data);
}

/*P2*/
static void* stack_pop(void **esp, int bytes)
{
  validate_data_ptr(*esp);
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
  printf ("%s: exit(%d)\n", t->name, exit_status);
  if (t->parent->child_waiting_on == t->tid) 
  {
    /* preemtion might occur here maybe it breaks shit*/
    sema_up(&t->parent->child_wait_sema);
  }
  /*lastly call process exit*/
  if(thread_current()->executable != NULL)
    file_close(thread_current()->executable);
  if(lock_held_by_current_thread(&fs_lock)){
    lock_release(&fs_lock);
  }
  t->our_child_self->exit = true;
  thread_exit();
  NOT_REACHED();
}

/*P2*/
/* given a user vaddr, returns a refernce the kernel can use */
/* in other words translates user to kernel addresses */
static void * user_to_kernel(void * va)
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
  struct an_open_file *threads_file = (struct an_open_file *)malloc(sizeof(struct an_open_file));

  if (file_struct == NULL) return -1; /* if the file doesnt exit return bad fd*/

  threads_file->file = file_struct;
  threads_file->file_descriptor = t->next_file_descriptor++; /* get the next fd and increment it after */

  list_push_back(&t->open_files,&threads_file->elem);

  return threads_file->file_descriptor;
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
 
/*P2*/
static void rm_file_from_open_files(int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for(e = list_begin(&t->open_files); e != list_end(&t->open_files); e=list_next(e))
  {
    struct an_open_file *an_open_f = list_entry(e, struct an_open_file, elem );
    int open_file_fd = an_open_f->file_descriptor;
    if(fd == open_file_fd)
    {
      /*remove the file with matching fd */
      list_remove(e);
    }
  }
}


