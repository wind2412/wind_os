/*
 * swap.h
 *
 *  Created on: 2017年6月3日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_SWAP_H_
#define INCLUDE_SWAP_H_

#include <types.h>
#include <x86.h>
#include <stdio.h>
#include <vmm.h>
#include <debug.h>

/**
 * 读取IDE盘。由于bootmain中自带，所以直接copy过来了（逃
 * 真实原因是由于bootmain隶属于bootloader，如果直接在其上改动的话，会让BootLoader超过512B。因此必须单独提取。
 */

#ifndef SECTSIZE
#define SECTSIZE 512
#endif

struct ide_t{
	u32 ideno;
	u32 cmdsets;
	u32 sect_num;
};

int waitdisk(void);

void swap_init();

void readsect(void *dst, u32 sectno, int ide_no, int sect_count);

void writesect(void *src, u32 sectno, int ide_no, int sect_count);

void swap_read(u32 dst_page_addr, struct pte_t *pte);

void swap_write(u32 src_page_addr, int sectno);

void swap_in(struct mm_struct *mm, u32 fault_addr, u32 flags);

void swap_out(struct mm_struct *mm, int n);


void test_swap();


#endif /* INCLUDE_SWAP_H_ */
