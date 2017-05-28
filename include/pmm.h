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

#define ROUNDUP(addr)	(addr - (addr % PAGE_SIZE) + PAGE_SIZE)
#define ROUNDDOWN(addr)	(addr - (addr % PAGE_SIZE))

#define get_outer_struct_ptr(node, type, member) ( (type *)( (u32)node - (u32)(&((type *)0)->member) ) )

struct Page{
	int ref;		//引用计数
	struct pte_t pt;
	struct list_node node;
};

struct free_area{
	struct list_node head;
	int free_page_num;
};

struct pmm_manager{
	const char *name;
	void (*init)();
	void (*init_page)();
	void (*alloc_page)(int);
	void (*free_page)();
};

void open_page_mm();

void print_memory();

void pmm_init();

void page_init();


#endif /* INCLUDE_PMM_H_ */
