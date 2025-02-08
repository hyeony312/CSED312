#include "vm/page.h"


extern struct lock f_lock;

void 
sp_table_init(struct hash *sp_table)
{
    hash_init(sp_table, sp_hash_func, sp_less_func, NULL);
}

static unsigned
sp_hash_func(const struct hash_elem *e,void *aux)
{
    struct sp_entry *temp = hash_entry(e, struct sp_entry, elem);

    return hash_int((int)(temp->vaddr));
}

static bool 
sp_less_func (const struct hash_elem *a, const struct hash_elem *b)
{
    struct sp_entry *temp1 = hash_entry(a, struct sp_entry, elem);
    struct sp_entry *temp2 = hash_entry(b, struct sp_entry, elem);

    return (temp1->vaddr < temp2->vaddr);
}

bool 
insert_spe (struct hash *sp_table, struct sp_entry *spe)
{
    bool result = false; 
    if (hash_insert(sp_table, &(spe->elem))==NULL) {
        result=true; 
    }
    return result; 
}

bool 
delete_spe (struct hash *sp_table, struct sp_entry *spe)
{
    if(hash_delete(sp_table, &spe->elem)==NULL) return false;

    return true;
}

struct 
sp_entry *find_spe (void *vaddr)  
{
    unsigned pg_num = pg_round_down(vaddr);
    struct sp_entry temp;
    temp.vaddr = pg_num;

    struct hash *table = &(thread_current()->sp_table);
    struct hash_elem *target = hash_find(table, &temp.elem);
    if(target!=NULL){
        return hash_entry(target, struct sp_entry, elem);
    }
    return NULL;
}

void 
spt_destroy (struct hash *sp_table)
{
    hash_destroy(sp_table, spt_destructor);
}

void
spt_destructor(struct hash_elem *elem, void *aux)
{
    struct sp_entry *temp; 

    temp = hash_entry(elem, struct sp_entry, elem);
    if(temp->is_loaded) // if page is loaded in physical memory 
    {
        free_page(pagedir_get_page(thread_current()->pagedir, temp->vaddr));
        pagedir_clear_page(thread_current()->pagedir, temp->vaddr); 
        free(temp); 
    }
    else if (temp->is_stack) // if page is in swap partition 
    {
        free_page(pagedir_get_page(thread_current()->pagedir, temp->vaddr));
        pagedir_clear_page(thread_current()->pagedir, temp->vaddr);
        bitmap_set(swap_bitmap, temp->swap_slot, false);
        free(temp); 
    }
}

struct frame_table_entry *
frame_alloc(enum palloc_flags flag)
{
    struct frame_table_entry *frame;
    void* kadd;

    kadd = palloc_get_page(flag);
    /* project 3 */
    while (kadd == NULL)
    {
       kadd = try_to_free_pages(flag); 
    }

    frame = malloc(sizeof(struct frame_table_entry));
    frame->kadd = kadd;
    frame->t = thread_current();

    list_push_back(&frame_table, &frame->f_elem);
    return frame;
}

bool load_file(void * kaddr, struct sp_entry *spe) {
    bool flag = !lock_held_by_current_thread(&f_lock);
    size_t check;

    if(flag) lock_acquire(&f_lock);

    check =  file_read_at(spe->file, kaddr, spe->read_bytes, spe->offset);
    if(flag) lock_release(&f_lock);

    if(check == spe->read_bytes) {
        memset(kaddr + check, 0, spe->zero_bytes);
        return true;
    }
    return false;
}
 
void
f_table_init(void)  
{
    list_init(&frame_table);
    current_clock = NULL;
}

void
free_page (void *kaddr)
{
    struct list_elem *fte_elem; 

    for(fte_elem=list_begin(&frame_table);fte_elem!=list_end(&frame_table);fte_elem=list_next(fte_elem))
    {
        struct frame_table_entry* fte_delete = list_entry(fte_elem, struct frame_table_entry, f_elem);
        if(fte_delete->kadd == kaddr){
            pagedir_clear_page(fte_delete->t->pagedir, fte_delete->spe->vaddr);
            list_remove(&fte_delete->f_elem);
            palloc_free_page(kaddr);
            break;
        }
    }
}

static struct list_elem*
get_next_frame()
{   
    struct list_elem *next; 
    // 1. current clock never been assigned 
    if(current_clock == NULL) 
    { 
        if(list_empty(&frame_table)) // if frame table empty 
        {return NULL; }
        
        next = list_begin(&frame_table); // returns list head -> next
    } 
    // 2. next have been assigned before
    else 
    {
        next = list_next(&current_clock->f_elem); 
        if(next == list_end(&frame_table)) 
        {   
            if (list_size(&frame_table)==1)// list에 원소 1개인 경우 
            { return NULL; }
            else // current clock was the last element of the list-> go back to the first frame 
            { next = list_begin(&frame_table); }
        }
    }
    current_clock = list_entry(next, struct frame_table_entry, f_elem); 
    return &current_clock->f_elem; 
}

void*
try_to_free_pages(enum palloc_flags flags) // evict 될 frame을 선택
{
    struct list_elem *elem; 
    struct frame_table_entry *evict_entry; 
    struct thread *thread;

    while(1)
    { // repeat until evict_entry is found 
        elem = get_next_frame(); // retrieve next frame from frame table 

        if(elem==NULL) // if elem==NULL, it means there is space in phyiscal memory 
        {    
            return palloc_get_page(flags); 
        }

        evict_entry = list_entry(elem, struct frame_table_entry, f_elem); 
        
        if (evict_entry->spe->is_stack==false) 
        {
            thread = evict_entry -> t; 

            // 1. if accessed bit is 1, set it to 0 (do not evict)
            if(pagedir_is_accessed(thread->pagedir, evict_entry->spe->vaddr)) 
            { 
                pagedir_set_accessed(thread->pagedir, evict_entry->spe->vaddr, false); 
            }
            // 2. if accessed bit is 0, evict 
            else 
            {   // VM BIN: if dirty, send to swap partition if modified 
                // VM FILE: if dirty, write back 
                // VM ANON: always send to swap partition 
                if(evict_entry->spe->type == VM_FILE)
                {
                    if(pagedir_is_dirty(thread->pagedir, evict_entry->spe->vaddr)) // if dirty, 
                    {
                        file_write_at(evict_entry->spe->file,evict_entry->kadd,evict_entry->spe->read_bytes,evict_entry->spe->offset); // write back 
                    }
                }
                else if (evict_entry->spe->type == VM_BIN)
                {
                    if(pagedir_is_dirty(thread->pagedir, evict_entry->spe->vaddr)) // if dirty, 
                    {
                        evict_entry->spe->type = VM_ANON; // change type to vm_anon 
                        evict_entry->spe->swap_slot = swap_out(evict_entry->kadd); // send to swap partition 
                    }
                }
                else if(evict_entry->spe->type == VM_ANON)
                {
                    evict_entry->spe->swap_slot = swap_out(evict_entry->kadd); // send to swap partition 
                }
                
                evict_entry->spe->is_loaded = false; // evict 
                pagedir_clear_page(thread->pagedir, evict_entry->spe->vaddr); // clear entry from page table 
                palloc_free_page(evict_entry->kadd); // deallocate physical memory  

                if (current_clock==evict_entry) // make sure evict entry is current clock 
                {
                    current_clock = list_entry(list_remove(&evict_entry->f_elem), struct frame_table_entry, f_elem);// move current clock to next 
                }

                free(evict_entry); // free evict_entry
                break; 
            }
        }
    }
    
return palloc_get_page(flags); 
}
