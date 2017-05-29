/*
 * malloc.h
 *
 *  Created on: 2017年5月28日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_MALLOC_H_
#define INCLUDE_MALLOC_H_

#include <types.h>
#include <stdio.h>
#include <pmm.h>

struct Chunk{
	u32 allocated:1;
	u32 size:31;
	struct list_node node;
};

void malloc_init();

void *malloc(u32 size);

void free(void *);

void test_malloc();


#endif /* INCLUDE_MALLOC_H_ */
