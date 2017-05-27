/*
 * default_pmm_manager.c
 *
 *  Created on: 2017年5月27日
 *      Author: zhengxiaolin
 */

#include <default_pmm_manager.h>

struct pmm_manager default_pmm_manager = {
	.name = "default",
	.init = default_pmm_init,
	.init_page = default_page_init,
};

void default_pmm_init()
{
	//初始化链表头 循环链表指向自身。
	extern struct free_area free_pages;
	list_init(&free_pages.head);
	free_pages.free_page_num = 0;
}

void default_page_init()
{

}
