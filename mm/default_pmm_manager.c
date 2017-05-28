/*
 * default_pmm_manager.c
 *
 *  Created on: 2017年5月27日
 *      Author: zhengxiaolin
 */

#include <default_pmm_manager.h>

extern struct free_area free_pages;

struct pmm_manager default_pmm_manager = {
	.name = "default",
	.init = default_pmm_init,
	.init_page = default_page_init,
	.alloc_page = default_alloc_page,
	.free_page = default_free_page,
};

void default_pmm_init()
{
	//初始化链表头 循环链表指向自身。
	list_init(&free_pages.head);
	free_pages.free_page_num = 0;
}

void default_page_init()
{
	/**
	 * 以下是设置内核二级页表映射的过程。
	 */
	extern u8 kern_start[];
	extern struct pde_t new_page_dir_t[];
	extern struct pte_t new_page_table_t[][PAGE_SIZE / sizeof(struct pte_t)];
	for(int i = (u32)kern_start >> 22; i < PAGE_DIR_NUM + ((u32)kern_start >> 22); i ++){		//开始设置页目录表的内容。假设内核一共128M大小。因此要设置128个页目录项。
		new_page_dir_t[i].sign = 0x07;
		new_page_dir_t[i].os 	= 0;
		new_page_dir_t[i].pt_addr = ((u32)new_page_table_t + (i - ((u32)kern_start >> 22)) * PAGE_SIZE - VERTUAL_MEM) >> 12;		//设置页表的地址，注意MMU要使用真实的地址，不是虚地址！！
	}
	u32 *pte = (u32 *)new_page_table_t;
	for(int i = 0; i < PAGE_DIR_NUM * PAGE_SIZE/sizeof(struct pte_t); i ++){	//开始设置内核页表的内容。
//		new_page_table_t[i/(PAGE_SIZE/sizeof(struct pte_t))][i%(PAGE_SIZE/sizeof(struct pte_t))].sign = 0x03;
//		new_page_table_t[i/(PAGE_SIZE/sizeof(struct pte_t))][i%(PAGE_SIZE/sizeof(struct pte_t))].os 	= 0;
//		new_page_table_t[i/(PAGE_SIZE/sizeof(struct pte_t))][i%(PAGE_SIZE/sizeof(struct pte_t))].page_addr = i;
		pte[i] = (i << 12) | 0x1 | 0x2;
	}
	//设置页表
	asm volatile ("movl %0, %%cr3"::"r"((u32)new_page_dir_t - VERTUAL_MEM));


	/**
	 * 以下是使用数据结构管理页面page的过程。
	 */
//	free_pages.free_page_num = PAGE_DIR_NUM * PAGE_SIZE/sizeof(struct pte_t);
//	u32 page_begin = pte_begin + PAGE_DIR_NUM * PAGE_SIZE;	//真正的页是从这里开始的。
//	printf("%x\n", page_begin);
}

void default_alloc_page(int size)
{

}

void default_free_page()
{

}
