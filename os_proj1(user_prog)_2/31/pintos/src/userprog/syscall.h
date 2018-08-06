#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

void	syscall_init (void);

void	halt(void);
void	exit(int status);
pid_t	exec(const char *cmd_line);
int		wait(pid_t pid);
int		read(int fd, void *buffer, unsigned size);
int		write(int fd, const void *buffer, unsigned size);
int		fibonacci(int n);
int		sum_of_four_integers(int a, int b, int c, int d);

// proj2-2
bool	create(const char *file, unsigned init_size);
bool	remove(const char *file);
int		open(const char *file);
int		filesize(int fd);
void	seek(int fd,unsigned position);
unsigned tell(int fd);
void	close(int fd);
 
#endif /* userprog/syscall.h */
