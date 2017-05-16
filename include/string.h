/*
 * string.h
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_STRING_H_
#define INCLUDE_STRING_H_

#include <types.h>

unsigned strlen(const char *ptr);

int strcmp(const char *lhs, const char *rhs);

void memset(void *src, u8 ch, u32 size);

char *strcpy(char *dest, const char *src);


#endif /* INCLUDE_STRING_H_ */
