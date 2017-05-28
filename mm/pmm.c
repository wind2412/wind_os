/*
 * pmm.c
 *
 *  Created on: 2017年5月23日
 *      Author: zhengxiaolin
 */

#include <pmm.h>

struct e820map *mm = (struct e820map *) ( 0x8000 + VERTUAL_MEM );

struct free_area free_pages;

struct pmm_manager *manager;

extern struct pmm_manager default_pmm_manager;

//u32 free_page_pool[]

void print_memory()
{
	for(int i = 0; i < mm->num; i ++){
		printf("base_address: %x  length: %x  type: %d\n", mm->map[i].base_lo, mm->map[i].length_lo, mm->map[i].type);
	}
}


//copy from hx_kernel
void page_fault(struct idtframe *frame)
{
    u32 cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2));

    printf("Page fault at EIP %x, virtual faulting address %x\n", frame->eip, cr2);
    printf("Error code: %x\n", frame->errorCode);

    // bit 0 为 0 指页面不存在内存里		//此处查阅资料，我在这里添加了U/S保护的确认。
    if ( !(frame->errorCode & 0x1)) {
        printf("Because the page wasn't present.\n");
    } else {		//如果errorCode & 0x1 == 1 则是：由于保护特权级别太高，造成无法读取，进而把bit1归0显示读错误。
    				//stackover flow: https://stackoverflow.com/questions/9759439/page-fault-shortage-of-page-or-access-violation
    	printf("The privilege of this page is so high～, you read it in user mode.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (frame->errorCode & 0x2) {
        printf("Write error.\n");
    } else {
        printf("Read error.\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (frame->errorCode & 0x4) {
        printf("In user mode.\n");
    } else {
        printf("In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (frame->errorCode & 0x8) {
        printf("Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (frame->errorCode & 0x10) {
        printf("The fault occurred during an instruction fetch.\n");
    }

    printf("esp: %x\n", frame->esp);

    while (1);
}

void pmm_init()
{
	set_handler(14, page_fault);		//设置页异常中断函数
	print_memory();
	page_init();
}

//不能这样。这样的话，这个“页目录表”是在内核中的。但是由于新的页表用于内存分配，即全是“空闲的”，因此必然会把内核的那一页空间置位P位为0不让分配出去。这样，内核不允许空闲页表读取了，
//从而也就不会被malloc分配出去了。但是！因为如果这样定义，页目录表自身在内核中！如果关闭了内核页，那么必然会造成页目录表无法读取！那么内存分配必然会崩溃了。这是一个矛盾的问题。
//因而ucore聪明地把所有内核外的空闲页取出第一页来当成页目录表，而且P位设置可用为1.这样可以读取了。但是要禁止把它分配出去。
struct pde_t new_page_dir_t[PAGE_SIZE/sizeof(struct pde_t)] __attribute__((aligned(PAGE_SIZE)));	//页目录占用一页4096B，每项4B，共计1024个页目录项(2^10)。每个页目录项管理1024个页表项(2^10)，每个页表占4096B(2^12)，因此能够管理4G内存。
struct pte_t new_page_table_t[PAGE_DIR_NUM][PAGE_SIZE / sizeof(struct pte_t)] __attribute__((aligned(PAGE_SIZE)));	//128*1024 也就是说，一共128个页表。

void page_init()
{
	for(int i = 0; i < mm->num; i ++){
		if(mm->map[i].type == 1 && i != 0){		//找到并非第一个空间
			extern u8 kern_end[];
			u32 free_mem_begin = (u32)kern_end;
			u32 free_mem_end = mm->map[i].base_lo + mm->map[i].length_lo;
			u32 pt_begin = ROUNDUP(free_mem_begin);		//页表的开始位置(内核本身不进行分页，仅仅分页空闲的空间作为malloc和free用)
			u32 pt_end = ROUNDDOWN(free_mem_end);		//页表的结束位置  但是这个变量其实是用不上的。因为分配不到这个位置。

			//使用default_pmm_manager
			manager = &default_pmm_manager;

			manager->init();
			manager->init_page();	//重新设置内核虚拟内存分页

//			while()

		}
	}
}
