/*
 * swap.c
 *
 *  Created on: 2017年6月3日
 *      Author: zhengxiaolin
 */

#include <swap.h>

struct ide_t swap = {
	.ideno = 1,
};

void waitdisk(void) {
	// Wait for disk ready.
	while ((inb(0x1F7) & 0xC0) != 0x40)
		;
}

void swap_init()
{
	//step 0: wait ready
	waitdisk();
	//step 1: select drive
	outb(0x1F6, 0xE0 | ((swap.ideno & 1) << 4));
	waitdisk();
	//step 2: send ATA identity command
	outb(0x1F7, 0xEC);
	waitdisk();

	//read identification space of this device
	u32 buffer[128];
	insl(0x1F0, buffer, sizeof(buffer)/sizeof(u32));			//这里出了问题！！没有除sizeof(u32)...导致全被清空了....
	swap.cmdsets = *(u32 *)((u32)buffer + 164);
	if(swap.cmdsets & 0x4000000){
		swap.sect_num = *(u32 *)((u32)buffer + 200);
	}else{
		swap.sect_num = *(u32 *)((u32)buffer + 120);
	}

	printf("swap's sect_num: %d\n", swap.sect_num);
}

// Read a single sector at offset into dst.		//offset is the sector number. --by wind.
void readsect(void *dst, u32 sectno, int ide_no, int sect_count)
{
	// Issue command.
	waitdisk();
	outb(0x1F2, sect_count);   // sect count
	outb(0x1F3, sectno);
	outb(0x1F4, sectno >> 8);
	outb(0x1F5, sectno >> 16);
	outb(0x1F6, (sectno >> 24) | 0xE0 | ((ide_no & 0x1) << 4) );
	outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

	// Read data.
	for(; sect_count > 0; sect_count --, dst += SECTSIZE){
		waitdisk();
		insl(0x1F0, dst, SECTSIZE / 4);
	}
}

void writesect(void *src, u32 sectno, int ide_no, int sect_count)
{
	// Issue command.
	waitdisk();
	outb(0x1F2, sect_count);   // sect count
	outb(0x1F3, sectno);
	outb(0x1F4, sectno >> 8);
	outb(0x1F5, sectno >> 16);
	outb(0x1F6, (sectno >> 24) | 0xE0 | ((ide_no & 0x1) << 4) );
	outb(0x1F7, 0x30);  // cmd 0x30 - write sectors

	// Read data.
	for(; sect_count > 0; sect_count --, src += SECTSIZE){
		waitdisk();
		outsl(0x1F0, src, SECTSIZE / 4);
	}
}

void swap_read(u32 dst_page_addr, struct pte_t *pte)
{
	readsect((void *)dst_page_addr, ((*(u32*)pte) >> 8) * PAGE_SIZE/SECTSIZE, 1, PAGE_SIZE/SECTSIZE);		//struct不能转为int。所以必须把struct*转为int*再解引用。
}

void swap_write(u32 src_page_addr, int sectno)
{
	writesect((void *)src_page_addr, sectno, 1, PAGE_SIZE/SECTSIZE);
}

//把页从磁盘给换进来。		fault_addr必然是va。
void swap_in(struct mm_struct *mm, u32 fault_addr)
{
	struct pte_t *wrong_pte = get_pte(mm->pde, fault_addr, 0);
	struct Page *page = alloc_page(1);
	swap_read(pg_to_addr_la(page), wrong_pte);		//向刚申请的page中，经由wrong_pte中存放的扇区号，然后读入磁盘的那一页，写到page上。
	//重新设置pte
//	wrong_pte->sign = 0x3;		//可能在用户模式下会有问题。		//要直接调用map
//	wrong_pte->page_addr = (pg_to_addr(page) >> 12);
	map(mm->pde, get_pg_addr_la(wrong_pte), fault_addr, 1);
	page->va = fault_addr;
	//把这个新换进来的页加入到FIFO的最后。
	list_insert_before(&mm->vm_fifo, &page->node);
}

void swap_out(struct mm_struct *mm, int n)
{
	struct list_node *begin = &mm->vm_fifo;
	while(begin->prev != &mm->vm_fifo && n > 0){
		struct Page *page = GET_OUTER_STRUCT_PTR(begin->next, struct Page, node);	//要被换出的页面。
		struct pte_t *pte = get_pte(mm->pde, page->va, 0);
		swap_write(pg_to_addr_la(page), ((page->va / PAGE_SIZE)+1) * PAGE_SIZE/SECTSIZE);		//ucore的+1非常漂亮！完美的解决了“不在磁盘中”和“第0个页面0x0～0x1000”高地址全都是0的冲突情况。
		pte->sign = 0;
		pte->page_addr = 0;
		*(u32 *)(&pte) = ((page->va / PAGE_SIZE + 1) << 8);		//按照交换磁盘page的方式设置pte的高24位。

		free_page(page, 1);
		begin = begin->prev;
		n --;

	}
}
