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

u32 malloc(u32 size);

void free(u32 addr);



#endif /* INCLUDE_MALLOC_H_ */
