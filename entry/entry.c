/*
 * entry.c
 *
 *  Created on: 2017年2月22日
 *      Author: zhengxiaolin
 */
#include <VGA.h>
#include <keyboard.h>
#include <stdio.h>
#include <pic.h>
#include <idt.h>
#include <gdt.h>
#include <timer.h>
#include <keyboard.h>

void init()
{
	clear_screen();
	putc('h');
	putc('e');
	putc('l');
	putc('l');
	putc('o');
	putc('\n');
	

	gdt_init();
	idt_init();
	pic_init();
//	timer_intr_init();
	kbd_init();

	//缺少scanf。中断完毕之后回来补。
//	int stat = inb(0x64);
//	printf("%d %c", stat, stat);
//	while((stat & 0x1) == 0);
//	putc(inb(0x60));
//	putc(inb(0x60));
//	putc(inb(0x60));
//
//	uint8_t c = getchar();
//	putc(c);

	while(1);
}
