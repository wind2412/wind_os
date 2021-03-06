/*
 * pmm.h
 *
 *  Created on: 2017年5月23日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_PMM_H_
#define INCLUDE_PMM_H_

#include <types.h>
#include <stdio.h>
#include <idt.h>
#include <list.h>
#include <string.h>
#include <debug.h>

struct e820map {			//from ucore lab2     bios probe
    int num;
    struct {
        u32 base_lo;
        u32 base_hi;
        u32 length_lo;
        u32 length_hi;
        u32 type;
    } __attribute__((packed)) map[20];
};

//页目录表 page directory
struct pde_t{
	u16 sign:9;
	u8  os:3;
	u32 pt_addr:20;
} __attribute__((packed));

//页表 page table
struct pte_t{
	u16 sign:9;
	u8  os:3;
	u32 page_addr:20;
} __attribute__((packed));

#define PAGE_SIZE 		4096
#define VERTUAL_MEM		0xC0000000

#define PAGE_DIR_NUM	128		//128个页目录项，因此共计能够映射128*2^10*2^12B = 512MB.

#define MAX_PAGE_NUM 	30000	//经过人工计算得到的页数量应该是三万+。

//#define ROUNDUP(addr)	(addr - (addr % PAGE_SIZE) + PAGE_SIZE)
//#define ROUNDDOWN(addr)	(addr - (addr % PAGE_SIZE))

//给宏变量加上括号是好习惯。防止输入算式的话，由于符号优先级会出错。
#define ROUNDUP(addr)	((addr) - ((addr) % PAGE_SIZE) + PAGE_SIZE)
#define ROUNDDOWN(addr)	((addr) - ((addr) % PAGE_SIZE))

#define GET_OUTER_STRUCT_PTR(node, type, member) ( (type *)( (u32)node - (u32)(&((type *)0)->member) ) )

struct Page{
	int ref;		//引用计数
	u32 flags;
	u32 free_pages;
	u32 va;		//用于fifo. //为什么这里要单独设置一个page的va呢？原因看下一句吧。有时候，page->va和pg_to_addr(page)并不同，这是因为：pg_to_addr只能转换“内核之后的空闲内存区”的地址！！而无法转换之前的！！如果我访问0x1000，那么根本无法转换！！所以ucore用了很巧妙的手法来解决这个问题。
	struct list_node node;
};

struct free_area{
	struct list_node head;
	int free_page_num;
};

struct Page *alloc_page(int n);

void free_page(struct Page *, int n);

u32 pg_to_addr_la(struct Page *page);

u32 pg_to_addr_pa(struct Page *page);

struct Page *la_addr_to_pg(u32 addr);

struct Page *pa_addr_to_pg(u32 addr);


struct pte_t *get_pte(struct pde_t *pde, u32 va, int is_create);

void map(struct pde_t *pde, u32 va, u32 pa, u8 bitsign);

void unmap(struct pde_t *pde, u32 va);

u32 get_pg_addr_la(struct pte_t * pte);

u32 get_pg_addr_pa(struct pte_t * pte);



void print_memory();

void pmm_init();

void page_init();


#endif /* INCLUDE_PMM_H_ */
