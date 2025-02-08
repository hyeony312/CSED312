#ifndef VM_PAGE_H
#define VM_PAGE_H
#include "lib/kernel/list.h"
#include "filesys/file.h"
#include "lib/kernel/hash.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "vm/swap.h"

#define VM_BIN 0
#define VM_FILE 1
#define VM_ANON 2

struct list frame_table;
struct frame_table_entry *current_clock;

struct frame_table_entry{
	void *kadd;
	struct sp_entry *spe;
	struct thread *t;
	struct list_elem f_elem;
};

struct sp_entry {
	uint8_t type;
	void *vaddr;
	bool writable;
	
	bool is_loaded;
	struct file *file;

    struct list_elem mmap_elem; // memory mapped file
	
	size_t offset;
	size_t read_bytes;
	size_t zero_bytes;

	size_t swap_slot; 

	bool is_stack;  
	
	struct hash_elem elem; // hash table element 
};

struct mmap_file{
	int mapid;
	struct file* file;
	struct list_elem elem;
	struct list spe_list;
};


void sp_table_init(struct hash *sp_table);
static unsigned sp_hash_func(const struct hash_elem *e,void *aux);
static bool sp_less_func (const struct hash_elem *a, const struct hash_elem *b);
bool insert_spe (struct hash *sp_table, struct sp_entry *spe);
bool delete_spe (struct hash *sp_table, struct sp_entry *spe);
struct sp_entry *find_spe (void *vaddr);
void spt_destroy (struct hash *sp_table);
void spt_destructor(struct hash_elem *elem, void *aux);
struct frame_table_entry *frame_alloc(enum palloc_flags flag);
bool load_file(void* kadd, struct sp_entry *spe);
void f_table_init(void); //1124
void free_page (void *kaddr); 
static struct list_elem* get_next_frame(); 
void* try_to_free_pages(enum palloc_flags flags);



#endif /* vm/page.h */