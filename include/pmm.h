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

struct e820map {			//from ucore lab2     bios probe
    int nr_map;
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

void open_page_mm();

void print_memory();

void init_pmm();


#endif /* INCLUDE_PMM_H_ */
