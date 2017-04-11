/*
 * entry.c
 *
 *  Created on: 2017年2月22日
 *      Author: zhengxiaolin
 */
#include <VGA.h>
#include <keyboard.h>
#include <stdio.h>

void init()
{
	clear_screen();
	putc('h');
	putc('e');
	putc('l');
	putc('l');
	putc('o');
	putc('\n');
	
	//缺少scanf。中断完毕之后回来补。
//	int stat = inb(0x64);
//	while((stat & 0x1) == 0);
//	putc(inb(0x60));
//	putc(inb(0x60));
//	putc(inb(0x60));
//
//	uint8_t c = getchar();
//	putc(c);

	while(1);
}
