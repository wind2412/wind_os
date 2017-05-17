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

	gdt_init();
	idt_init();
	pic_init();
	timer_intr_init();

	sti();		//疑惑：不需要调用sti开中断吗？？？	莫非pic的mask全变成0相当于开了中断了吗？

//	asm volatile ("cli");	//再使用cli就并不好使了？？	并不。这里是禁止了硬件中断。但是像是int 0x80这样的软中断还是可以使用的。
	asm volatile ("int $0x03");
	asm volatile ("int $0x04");

	while(1);
}
