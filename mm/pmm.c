/*
 * pmm.c
 *
 *  Created on: 2017年5月23日
 *      Author: zhengxiaolin
 */

#include <pmm.h>

struct e820map *mm = (struct e820map *) ( 0x8000 + VERTUAL_MEM );

struct free_area free_pages;

u32 free_page_pool[MAX_PAGE_POOL];

int page_stack_top = -1;		//模仿hx的栈实现

struct list_node *prev_alloc = NULL;		//全局量，前一个alloc页。	多进程的话，这里应该设置互斥量。


void add_page_addr_to_stack(u32 page_addr)
{
	free_page_pool[++ page_stack_top] = page_addr;
}

struct Page alloc_page()
{
	if(page_stack_top < 0)	panic("stack underflow!");
	struct Page page = {
			.ref = 1,
			.va = free_page_pool[page_stack_top --] + VERTUAL_MEM,
			.free_mem = PAGE_SIZE,
			.node = {
				.prev = prev_alloc,
				.next = NULL,
			},
	};
	list_insert_before(&free_pages.head, &page.node);
	prev_alloc = &page.node;
	free_pages.free_page_num -= 1;
	return page;
}

void free_page(struct Page *page)
{
	free_page_pool[++ page_stack_top] = page->va - VERTUAL_MEM;
	if(prev_alloc == &page->node)	prev_alloc = page->node.prev;		//如果free的是登记在全局量的alloc页，那么如果不修改登记的全局量，会影响到alloc_page()中链表的前驱节点记录。
	list_delete(&page->node);
	free_pages.free_page_num += 1;
}

void map(u32 va, u32 pa, int is_user)
{
	extern struct pde_t *pd;
	extern u8 kern_start[];
	if(va > 0xC0100000 && va < (u32)kern_start)					return;		//不允许link内核区域（笑
	if(va > 0x100000   && va < (u32)kern_start - VERTUAL_MEM)	return;		//不允许link内核区域（笑 按理来说，pa也应该设定，但是还是算了。

	if(pd[va >> 22].pt_addr == 0){	//如果va没有被页处理  如果是已经处理，但是却被free掉了，这里应该会出页异常。hx这里做的很不好，或许因为太简单了吧。

		u32 new_page = alloc_page().va;

		//设置页目录表
		pd[va >> 22].sign = 0x7;
		pd[va >> 22].os	  = 0;
		pd[va >> 22].pt_addr = new_page >> 12;

		memset((struct pte_t *)new_page, 0, PAGE_SIZE);

		//设置页表
		if(is_user) ((struct pte_t *)new_page)[va >> 12].sign = 0x7; else ((struct pte_t *)new_page)[va >> 12].sign = 0x3;
		((struct pte_t *)new_page)[va >> 12].os	  = 0;
		((struct pte_t *)new_page)[va >> 12].page_addr = pa >> 12;

	} else {

		//设置页表
		u32 pte = (pd[va >> 22].pt_addr << 12) + VERTUAL_MEM;
		if(is_user) ((struct pte_t *)pte)[va >> 12].sign = 0x7; else ((struct pte_t *)pte)[va >> 12].sign = 0x3;
		((struct pte_t *)pte)[va >> 12].os	  = 0;
		((struct pte_t *)pte)[va >> 12].page_addr = pa >> 12;

	}

	asm volatile ("invlpg (%0)"::"r"(va));		//刷新TLB.
}

void unmap(u32 va)
{
	extern struct pde_t *pd;
	u32 pte = (pd[va >> 22].pt_addr << 12) + VERTUAL_MEM;

	((struct pte_t *)pte)[va >> 12].sign = 0;
	((struct pte_t *)pte)[va >> 12].os   = 0;
	((struct pte_t *)pte)[va >> 12].page_addr = 0;

	asm volatile ("invlpg (%0)"::"r"(va));		//刷新TLB.
}

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
//struct pde_t new_page_dir_t[PAGE_SIZE/sizeof(struct pde_t)] __attribute__((aligned(PAGE_SIZE)));	//页目录占用一页4096B，每项4B，共计1024个页目录项(2^10)。每个页目录项管理1024个页表项(2^10)，每个页表占4096B(2^12)，因此能够管理4G内存。
//struct pte_t new_page_kern_table[2 * PAGE_SIZE / sizeof(struct pte_t)] __attribute__((aligned(PAGE_SIZE)));	//假设我们的内核只有8个M。两页就能放下。
struct pte_t new_page_freemem_table[PAGE_DIR_NUM * PAGE_SIZE / sizeof(struct pte_t)] __attribute__((aligned(PAGE_SIZE)));	//128*1024 也就是说，一共128个页表。

u32 pt_begin;		//空闲空间的起始位置

void page_init()
{

	for(int i = 0; i < mm->num; i ++){
		if(mm->map[i].type == 1 && i != 0){		//找到并非第一个空间
			extern u8 kern_end[];
			u32 free_mem_begin = (u32)kern_end - VERTUAL_MEM;
			u32 free_mem_end = mm->map[i].base_lo + mm->map[i].length_lo;
			pt_begin = ROUNDUP(free_mem_begin);		//空闲页的开始位置(内核本身不进行分页，仅仅分页空闲的空间作为malloc和free用)
			u32 pt_end = ROUNDDOWN(free_mem_end);		//空闲页的结束位置

			printf("=====%x=====\n", pt_begin);//0x1af000

			for(u32 i = pt_begin; i < pt_end; i += PAGE_SIZE){
				add_page_addr_to_stack(i);
			}

			//初始化数据结构
			list_init(&free_pages.head);
			free_pages.free_page_num = page_stack_top;

			//重新分页!
			extern struct pde_t *pd;
			extern struct pte_t *snd;
			//原先已经有过的，保持原样就可以，不用在设置了。
			//把内核结束开始的新一页(free_mem page空间)到所有空闲结束的所有映射。	//原先0xC0000000~0xC0400000的已经全部映射了。我们从0xC0400000开始吧。
			for(int i = ((u32)(pt_begin + VERTUAL_MEM) >> 22) + 1; i < ((u32)(pt_end + VERTUAL_MEM) >> 22); i ++){
				pd[i].pt_addr = ((u32)&new_page_freemem_table[PAGE_SIZE * i] - VERTUAL_MEM) >> 12;
				pd[i].os = 0;
				pd[i].sign = 0x07;
			}
			for(int i = 0; i < PAGE_DIR_NUM * PAGE_SIZE / sizeof(struct pte_t); i ++){
				new_page_freemem_table[i].page_addr = i + 1024;
				new_page_freemem_table[i].os = 0;
				new_page_freemem_table[i].sign = 0x03;
			}
			//设置页表
			asm volatile ("movl %0, %%cr3"::"r"(pd));
		}
	}
}
