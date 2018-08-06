#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/off_t.h"
#include "filesys/filesys.h"
#include "threads/synch.h"

#define FILE_MAX 128
struct semaphore sema_write;
struct semaphore sema_read;

static void syscall_handler (struct intr_frame *);
void check_memory_valid(void *addr);
void * get_arg (void *ptr );

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f UNUSED) 
{

  void *syscall_num = (f->esp);
  void *arg1, *arg2, *arg3, *arg4;
  check_memory_valid(syscall_num+3);

  switch( *(int*)syscall_num ){
	case SYS_HALT:
	  halt();
	  break;
	case SYS_EXIT:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);

	  // void exit(int status)
	  exit( *(int *)arg1 ); 

	  break;
	case SYS_EXEC:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);
	  
	  //f->eax = (uint32_t)exec( (char *)arg1 );
	  f->eax = (uint32_t)exec( (char *)*(int*)arg1 );

	  break;
	case SYS_WAIT:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);

	  // int wait(pid_t pid)
	  f->eax = (uint32_t)process_wait( *(int *)arg1 );
	  break;
	case SYS_READ:
	  arg1 = f->esp + 4*1;
	  arg2 = f->esp + 4*2;
	  arg3 = f->esp + 4*3;
	  check_memory_valid(arg3+3);

	  // int read(int fd, void *buffer, unsigned size)
	  lock_acquire(&filesys_lock);
	  f->eax = read( *(int *)arg1, *(void **)arg2, *(unsigned int*)arg3 );
	  lock_release(&filesys_lock);

	  break;
	case SYS_WRITE:
	  arg1 = f->esp + 4*1;
	  arg2 = f->esp + 4*2;
	  arg3 = f->esp + 4*3;
	  check_memory_valid(arg3+3);

	  // int write(int fd, void *buffer, unsigned size)
	  lock_acquire(&filesys_lock);
	  f->eax = write( *(int *)arg1, *(void **)arg2, *(unsigned int*)arg3 );
	  lock_release(&filesys_lock);

	  break;
	case SYS_FIBO:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);

	  f->eax = fibonacci( *(int *)arg1 );
	  break;
	case SYS_SUMINT:
	  arg1 = f->esp + 4*1;
	  arg2 = f->esp + 4*2;
	  arg3 = f->esp + 4*3;
	  arg4 = f->esp + 4*4;
	  check_memory_valid(arg4+3);

	  f->eax = sum_of_four_integers( *(int *)arg1, *(int *)arg2, 
									 *(int *)arg3, *(int *)arg4);
	  break;
	case SYS_CREATE:
	  arg1 = f->esp + 4*1;
	  arg2 = f->esp + 4*2;
	  check_memory_valid(arg2+3);
	
	  // maybe chagne
	  f->eax = create((char *)*(int*)arg1, *(unsigned int*)arg2 );
	  break;
	case SYS_REMOVE:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);
	  
	  lock_acquire(&filesys_lock);
	  f->eax = remove( (char *)*(int*)arg1 );
	  lock_release(&filesys_lock);
	  break;
	case SYS_OPEN:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);

	  lock_acquire(&filesys_lock);
	  f->eax = open( (char *)*(int*)arg1);
	  lock_release(&filesys_lock);
	  break;
	case SYS_CLOSE:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);
	  
	  lock_acquire(&filesys_lock);
	  close( *(int*)arg1 );
	  lock_release(&filesys_lock);
	  break;
	case SYS_FILESIZE:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);
	  
	  lock_acquire(&filesys_lock);
	  f->eax = filesize( *(int*)arg1 );
	  lock_release(&filesys_lock);
	  break;
	case SYS_TELL:
	  arg1 = f->esp + 4*1;
	  check_memory_valid(arg1+3);
	  
	  lock_acquire(&filesys_lock);
	  f->eax = tell( *(int*)arg1 );
	  lock_release(&filesys_lock);
	  break;
	case SYS_SEEK:
	  arg1 = f->esp + 4*1;
	  arg2 = f->esp + 4*2;
	  check_memory_valid(arg2+3);

	  // maybe change
	  lock_acquire(&filesys_lock);
	  seek( *(int*)arg1, *(unsigned*)arg2 );
	  lock_release(&filesys_lock);
	  break;
	  
	default:
	  break;
  }

}

void 
halt (void)
{
  shutdown_power_off();
}

void  
exit (int status)
{
  struct thread *cur = thread_current();
  cur->exit_status = status;

  char cur_name[16+1];
  int ii = 0;
  
  memset(cur_name, 0x00, sizeof(cur_name) );
  for( ii = 0; ii < strlen(cur->name); ii++ ){
	if( cur->name[ii] == '\0' || cur->name[ii] == ' ')
	  break;
	cur_name[ii] = cur->name[ii];
  }
  cur_name[ii] = '\0';


  for( ii = 0; ii < FILE_MAX; ii++){
	if( cur->of_info[ii].fp != NULL )
	  close( cur->of_info[ii].fp);
		
  }

  printf("%s: exit(%d)\n", cur_name, cur->exit_status); 
  thread_exit();

}

pid_t 
exec (const char *cmd_line)
{
  return process_execute(cmd_line);
}

int 
wait (pid_t pid)
{
  return process_wait(pid);
}

int
read (int fd, void *buffer, unsigned size)
{
  int result;
  int ii;
  char temp;
  struct thread *cur = thread_current();
  
  if( !is_user_vaddr(buffer) || !is_user_vaddr(buffer+size-1) )
	exit(-1);

  switch(fd){
	case 0:			// fd_read
	  for ( ii = 0; ii< size; ii++){
		temp = input_getc();
		*(unsigned char*)(buffer+ii) = temp;

		if ( temp == '\0' || temp == '\n'){
		  *((char*)(buffer+ii)) = '\0';
		  break;		  
		}
	  }
	  return ii;
	case 1:			// fd_write
	case 2:			// fd_error
	  return -1;	
	case 3:
	default:
	  //lock_acquire(&filesys_lock);
	  for( ii = 0 ; ii < FILE_MAX; ii++){
		if( cur->of_info[ii].fd == fd ){
		  result = file_read(cur->of_info[ii].fp, buffer, size);
		  return result;
		}
	  }
	  exit(-1); 
  }
}

int 
write (int fd, const void *buffer, unsigned size)
{
  int ii;
  int result;
  struct thread *cur = thread_current();
  switch(fd){
	case 2:
	case 0:			// fd_read
	  return -1; 
	case 1:
	  putbuf(buffer, size);
	  return size;
	case 3:
	default:
	  //lock_acquire(&filesys_lock);
	  for(ii = 0; ii < FILE_MAX; ii++){
		if( cur->of_info[ii].fd == fd ){
		  result = file_write(cur->of_info[ii].fp, buffer, size);
		  return result;
		}
	  }
	  exit(-1);
  }

}

int 
fibonacci (int n)
{
  int n1 = 1, n2 = 1;
  int i, temp;

  if( n ==1 || n == 2 ){
	return 1;
  }

  for( i = 3; i <= n; i++ ){
	temp = n1;
	n1 = n2;
	n2 = n1 + temp;
  }
  return n2;		
}

int 
sum_of_four_integers (int a, int b, int c, int d)
{
  return a+b+c+d;
}

void
check_memory_valid (void *addr)
{
  if( addr < 0x08048000 || addr >= PHYS_BASE ){
	exit(-1);
  }
}

bool    
create (const char *file, unsigned init_size)
{
  if( file == NULL)
	exit(-1);
  return filesys_create(file, init_size);  
}

bool    
remove (const char *file)
{
  if( file == NULL)
	exit(-1);
  return filesys_remove(file); 
}

int     
open (const char *file)
{
  static fd = 2;
  int ii;
  struct file *fp;
  struct thread *cur = thread_current();

  fd++;  

  if( file == NULL) {
	return -1;
  }

  fp = filesys_open(file);

  if( fp == NULL){
	return -1;
  }

  else{
	for( ii = 0; ii < FILE_MAX; ii++){

	  if( cur->of_info[ii].fp == NULL ){
		cur->of_info[ii].fp = fp;
		cur->of_info[ii].fd = fd;
		return fd;
	  }
	}
	return -1;
  }
}

int     
filesize (int fd)
{
  int ii;
  struct thread *cur = thread_current();

  for( ii = 0 ; ii < FILE_MAX; ii++){

	if( cur->of_info[ii].fd == fd ){
	  if( cur->of_info[ii].fp == NULL)
		exit(-1);
	  
	  return file_length(cur->of_info[ii].fp);
	}
  }
}

void    
seek (int fd,unsigned position){
  
  int ii;
  struct thread *cur = thread_current();

  for( ii = 0; ii < FILE_MAX; ii++){
	if( cur->of_info[ii].fd == fd ){
	  if( cur->of_info[ii].fp == NULL)
		exit(-1);
	  
	  file_seek(cur->of_info[ii].fp, position);
	}
  }
}

unsigned 
tell (int fd){

  int ii;
  struct thread *cur = thread_current();

  for( ii = 0; ii < FILE_MAX; ii++){
	if( cur->of_info[ii].fd == fd ){
	  if( cur->of_info[ii].fp == NULL)
		exit(-1);
	  
	  return file_tell(cur->of_info[ii].fp);
	}
  }

}

void    
close (int fd){

  int ii;
  struct file* fp;
  struct thread *cur = thread_current();

  for( ii = 0; ii < FILE_MAX; ii++){
	if( cur->of_info[ii].fd == fd ){
	  if( cur->of_info[ii].fp == NULL)
		exit(-1);
	  
	  fp = cur->of_info[ii].fp;
	  cur->of_info[ii].fp = NULL;
	  file_close(fp);
	  
	}
  }
}


