/*
 * pmm.c
 *
 *  Created on: 2017年5月23日
 *      Author: zhengxiaolin
 */

#include <pmm.h>

struct e820map *mm = (struct e820map *)0x8000;

void print_memory()
{
	for(int i = 0; i < mm->nr_map; i ++){
		printf("base_address: %x  length: %x  type: %d\n", mm->map[i].base_lo, mm->map[i].length_lo, mm->map[i].type);
	}
}

void init_pmm()
{
	print_memory();
}
