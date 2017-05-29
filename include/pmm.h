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

#define MAX_PAGE_POOL 	40000	//经过人工计算得到的页数量应该是三万+。

#define ROUNDUP(addr)	(addr - (addr % PAGE_SIZE) + PAGE_SIZE)
#define ROUNDDOWN(addr)	(addr - (addr % PAGE_SIZE))

#define GET_OUTER_STRUCT_PTR(node, type, member) ( (type *)( (u32)node - (u32)(&((type *)0)->member) ) )

struct Page{
	int ref;		//引用计数
	u32 va;
	u32 free_mem;
	struct list_node node;
};

struct free_area{
	struct list_node head;
	int free_page_num;
};

void add_page_addr_to_stack(u32 page);

struct Page alloc_page();

void free_page(struct Page *);

void map(u32 va, u32 pa, int is_user);

void unmap(u32 va);

void open_page_mm();

void print_memory();

void pmm_init();

void page_init();


#endif /* INCLUDE_PMM_H_ */
