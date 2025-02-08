#ifndef VM_SWAP_H
#define VM_SWAP_H


#include "devices/block.h"
#include "threads/synch.h"
#include <bitmap.h>
#include "threads/vaddr.h"
#include <stdio.h>
#include <stddef.h>
#include "lib/stddef.h"
#include <stdbool.h>
#include "userprog/process.h"
#include <bitmap.h>

struct lock swap_lock; 
struct block *swap_block; 
struct bitmap *swap_bitmap; 

void init_swap();
void swap_in(size_t used_index, void* kaddr);
size_t swap_out(void* kaddr); 

#endif  