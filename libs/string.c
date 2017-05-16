/*
 * string.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <string.h>

unsigned strlen(const char *ptr){
	if(ptr == NULL)	return 0;
	unsigned len = 0;
	while(*ptr != '\0'){
		len ++;
		ptr ++;
	}
	return len;
}

int strcmp(const char *lhs, const char *rhs){
	if(lhs == NULL || rhs == NULL)	return 0;
	while(*lhs != '\0' && *rhs != '\0'){
		if(*lhs++ == *rhs++){
			continue;
		}else{
			return *lhs - *rhs;
		}
	}
	return *lhs - *rhs;
}

char *strcpy(char *dest, const char *src){
	if(dest == NULL || src == NULL)	return NULL;
	char *temp = dest;
	while(*src != '\0'){
		*dest++ = *src++;
	}
	*dest = '\0';
	return temp;
}

void memset(void *src, u8 ch, u32 size){
	char *ptr = (char *)src;		//memset 是每隔8位一设置的。
	while(size-- > 0){
		*ptr++ = ch;
	}
}

