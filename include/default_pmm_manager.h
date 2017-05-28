/*
 * default_pmm_manager.h
 *
 *  Created on: 2017年5月27日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_DEFAULT_PMM_MANAGER_H_
#define INCLUDE_DEFAULT_PMM_MANAGER_H_

#include <pmm.h>

void default_pmm_init();

void default_page_init();

void default_alloc_page(int size);

void default_free_page();

#endif /* INCLUDE_DEFAULT_PMM_MANAGER_H_ */
