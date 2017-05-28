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
	//内核页表已经设置过了，不再设置了。日狗。




}

void default_alloc_page(int size)
{

}

void default_free_page()
{

}
