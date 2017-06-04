/*
 * vmm.h
 *
 *  Created on: 2017年6月2日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_VMM_H_
#define INCLUDE_VMM_H_

#include <types.h>
#include <list.h>
#include <pmm.h>
#include <malloc.h>
#include <stdio.h>

/**
 * 思路来源于ucore。
 */

struct mm_struct{
	struct pde_t *pde;
	struct vma_struct *cache;
	u32 num;
	struct list_node node;
	struct list_node vm_fifo;		//页替换算法的链表  即victim Page的结构体链表
};

struct vma_struct{
	u32 vmm_start;
	u32 vmm_end;
	struct mm_struct *back_link;
	struct list_node node;
};

void vmm_init();

struct mm_struct *create_mm(struct pde_t *pde);

struct vma_struct *create_vma(struct mm_struct *mm, u32 vmm_start, u32 vmm_end);

//返回vmm_start比之大的第一个vma
struct vma_struct *find_vma(struct mm_struct *mm, u32 addr);

void free_vma(struct vma_struct *addr);

void free_mm(struct mm_struct *addr);


void page_fault(struct idtframe *frame);

void do_swap(u32 cr2);

#endif /* INCLUDE_VMM_H_ */
