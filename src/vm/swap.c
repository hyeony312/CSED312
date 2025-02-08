#include "vm/swap.h"

extern struct lock f_lock;

void
init_swap() 
{
    swap_block = block_get_role(BLOCK_SWAP); 
    lock_init(&swap_lock); 
    /* PROJECT 3: block_size 함수가 num of sector를 반환하기 때문에 PGSIZE가 아닌 PGSIZE/SECTOR SIZE로 나눠줘야 함*/

    swap_bitmap = bitmap_create(block_size(swap_block) / 8);
    bitmap_set_all (swap_bitmap, 0); // 비어있을 때 0, 차있을 때 1 
}

void 
swap_in(size_t used_index, void* kaddr)
{
    // disk에서 필요한 페이지를 가져오려고 하는데, 해당 disk가 비어있는 경우 종료   
    if(bitmap_test(swap_bitmap, used_index) == 0) return;

    int unit_sector = PGSIZE / BLOCK_SECTOR_SIZE; 
    

    //lock_acquire(&f_lock);
    for (int i = 0 ; i < unit_sector ; i++)
    {
        block_read (swap_block, used_index * unit_sector + i, kaddr + i * BLOCK_SECTOR_SIZE);
    }
    //lock_release(&f_lock);

    bitmap_set(swap_bitmap, used_index, 0); // 읽어들인 block은 free하기 
}

size_t 
swap_out(void* kaddr)
{
    int begin_index = bitmap_scan_and_flip(swap_bitmap, 0, 1, 0); // 비트맵에서 값이 0인 연속 1칸을 찾아서 1으로 flip 

    if (begin_index == BITMAP_ERROR) return BITMAP_ERROR;

    int unit_sector = PGSIZE / BLOCK_SECTOR_SIZE; 
    
    //lock_acquire(&f_lock);
    for (int i = 0 ; i < unit_sector ; i++)
    {
        block_write (swap_block, begin_index * unit_sector + i, kaddr + i * BLOCK_SECTOR_SIZE);
    }
    //lock_release(&f_lock);

    return begin_index;
}