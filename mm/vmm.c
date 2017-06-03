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
	list_init(&mm->vm_fifo);
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
	if(mm->cache->vmm_start < addr && mm->cache->vmm_end > addr)	return mm->cache;		//从cache中试着读取一波（局部性原理）
	struct list_node *begin = &mm->node;
	while(begin->next != &mm->node){
		struct vma_struct *temp = GET_OUTER_STRUCT_PTR(begin->next, struct vma_struct, node);
		if(temp->vmm_start < addr && temp->vmm_end > addr){
			mm->cache = temp;
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

//copy from hx_kernel
void page_fault(struct idtframe *frame)
{
    u32 cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2));

    printf("Page fault at EIP %x, virtual faulting address %x\n", frame->eip, cr2);
    printf("Error code: %x\n", frame->errorCode);

    // bit 0 为 0 指页面不存在内存里
    if (frame->errorCode & 0x4) {
        printf("In user mode.\n");
    } else {
        printf("In kernel mode.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (frame->errorCode & 0x2) {
        printf("Write error.\n");
    } else {
        printf("Read error.\n");
    }

    switch (frame->errorCode & 0x3) {
    	case 0:
    		//read一个not present的页面，  这样应该从磁盘拿来新的页面
    		do_swap(cr2);
    		break;
    	case 1:
    		//read一个present的页面 竟然还能出错？
    		if((frame->errorCode & 0x4) == 1){
    			//此处查阅资料，我在这里添加了U/S保护的确认。
    			//如果errorCode & 0x1 == 1 则是：由于保护特权级别太高，造成无法读取，进而把bit1归0显示读错误。
    			//stackover flow: https://stackoverflow.com/questions/9759439/page-fault-shortage-of-page-or-access-violation
    			printf("The privilege of this page is so high～, you read it in user mode.\n");
    			while(1);		//宕机
    		}else{
    			printf("cannot be there...\n");
    			while(1);
    		}
    		break;
    	case 2:
    		//write一个not present的页面， 这样应该从磁盘拿来新的页面
    		do_swap(cr2);
    		break;
    	case 3:
    		//write一个present的页面，页保护异常。这样应该COW。 由于最后两位00000011b，因此正在写，而且页面存在。但是又出了异常，因而原先的pte必然是只读的。可以推理出来。
    		printf("COW is not supported...\n");
    		while(1);
    		//具体实现先行搁置吧。？？？？？？？？？？？？？？？？？？？？？？？？？？？

    		if((frame->errorCode & 0x4) == 1){
    			printf("The privilege of this page is so high～, you read it in user mode.\n");
    			while(1);
    		}

    		break;
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

}

void do_swap(u32 cr2)
{
	u32 fault_pg_addr = ROUNDDOWN(cr2);

	struct pte_t *pte = get_pte(mm->pde, fault_pg_addr, 1);	//1.如果页目录表有的话，那么返回。
															//2.如果页目录表还没有的话，说明正在指定访问一个比较偏的内存位置。需要新建立一个目录表项，通过申请一个页。
	if(pte->page_addr == 0 && pte->sign == 0){		//1,2->如果页表也没有的话，那就只能另申请一个页来存放页表了。(看来访问的地址确实挺偏的)
		struct Page *pt_pg = alloc_page(1);
		if(pt_pg == NULL)	return;	//panic更好
		pt_pg->va = fault_pg_addr;		//新申请页表的所在页要和访问的目的页的page->va使用同一个fault_pg_addr！这里一定要注意。因为这个va会在swap_out中使用.
		map(mm->pde, fault_pg_addr, pg_to_addr_pa(pt_pg), 1);
		//把新alloc的page加到vm_fifo列表中
		list_insert_before(&mm->vm_fifo, &pt_pg->node);
	}

	swap_in(mm, fault_pg_addr);

}
