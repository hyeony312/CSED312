#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
//test
#include "threads/interrupt.h"
#include "userprog/syscall.h"
#include "vm/page.h"
#include "vm/swap.h"


tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

/*project2*/
void arg_stk(char ** argv, uint32_t argc, struct intr_frame * if_);
//void passing_argument(char **arguments, int count, char **esp);
struct thread *search_pid(int pid);
void delete_child(struct thread *child);

int process_add_fdTable (struct file *f);
struct file *process_search_fdTable(int fd);
void process_remove_fdTable (int fd);
bool page_fault_handler(struct sp_entry *spe);

// extern struct lock filesys_lock; //project 3 lock
bool expand_stack(void * vaddr);
extern struct lock filesys_lock; // project 3 
#endif /* userprog/process.h */
