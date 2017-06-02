/*
 * vmm.c
 *
 *  Created on: 2017年6月2日
 *      Author: zhengxiaolin
 */

#include <vmm.h>

struct mm_struct *mm;		//全局的mm

struct mm_struct *create_mm(struct pde_t *pde)
{
	struct mm_struct * mm = (struct mm_struct *)malloc(sizeof(struct mm_struct));
	mm->num = 0;
	mm->pde = pde;
	mm->cache = NULL;
	list_init(&mm->node);
	return mm;
}

struct vma_struct *create_vma(struct mm_struct *mm, u32 vmm_start, u32 vmm_end)
{
	struct vma_struct *vma = (struct vma_struct *)malloc(sizeof(struct vma_struct));
	vma->vmm_start = vmm_start;
	vma->vmm_end = vmm_end;
	vma->back_link = mm;

	//insert
	struct list_node *begin = &mm->node;
	while(begin->next != &mm->node){
		struct vma_struct *temp = GET_OUTER_STRUCT_PTR(begin->next, struct vma_struct, node);
		if(temp->vmm_start > vmm_start){	//找到第一个比要插入点大的点 并且插入到右边 因为这时候begin还没有走到next，只是用next来判断的。
			list_insert_after(begin, &vma->node);
			mm->num ++;
			break;
		}
		begin = begin->next;
	}
	if(begin == &mm->node){
		list_insert_after(&mm->node, &vma->node);
		mm->num ++;
	}
	return vma;
}

struct vma_struct *find_vma(struct mm_struct *mm, u32 addr)
{
	struct list_node *begin = &mm->node;
	while(begin->next != &mm->node){
		struct vma_struct *temp = GET_OUTER_STRUCT_PTR(begin->next, struct vma_struct, node);
		if(temp->vmm_start < addr && temp->vmm_end > addr){
			return temp;
		}
	}
	return NULL;
}

void free_vma(struct vma_struct *vma)
{
	free(vma);
}

void free_mm(struct mm_struct *mm)
{
	struct list_node *prev = (&mm->node)->next;
	struct list_node *next = prev->next;
	while(prev != &mm->node){	//后边指针next不是头
		struct vma_struct *temp = GET_OUTER_STRUCT_PTR(prev, struct vma_struct, node);
		list_delete(&temp->node);
		free_vma(temp);			//此函数或许bug？？？？

		prev = next;
		next = prev->next;
	}
	free(mm);
}
