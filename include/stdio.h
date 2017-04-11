/*
 * stdio.h
 *
 *  Created on: 2017年2月26日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

/**************  printf  ****************/
void putc(char c);
void put_raw_string(const char *c);
void put_int(uint32_t num, uint32_t base);
void printf(const char *fmt, ...);

/***************  scanf  ****************/
#define 	BUF_MAXSIZE		256

char buf[BUF_MAXSIZE];




#endif /* INCLUDE_STDIO_H_ */
