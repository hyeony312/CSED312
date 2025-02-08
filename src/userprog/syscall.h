#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdbool.h>
#include "filesys/off_t.h"
#include "vm/page.h"
#include "lib/user/syscall.h"
#include "userprog/pagedir.h"

#define CLOSE_ALL -9999

void syscall_init (void);

struct sp_entry *if_user_add(void *addr);
void syscall_exit(int status);
bool syscall_create(const char *file , unsigned int size);
bool syscall_remove(const char *file);
int syscall_exec(const *cmd_line);
int syscall_wait(int child_tid);
int syscall_open(const char *file);
int syscall_filesize (int fd);
int syscall_read (int fd, void *buffer, unsigned size);
int syscall_write(int fd, void *buffer, unsigned size);
void syscall_seek (int fd, off_t position);
off_t syscall_tell (int fd);
void syscall_close (int fd);

/*project 3*/
void is_valid_buffer(void* buffer, unsigned size, void *esp, bool writable);
void is_valid_str(const char *str);
void syscall_munmap(int mapping);
int syscall_mmap(int fd, void* addr);

#endif /* userprog/syscall.h */
