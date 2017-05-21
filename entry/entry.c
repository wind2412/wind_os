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
#include <tss.h>
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
	tss_init();
	outb(0x21, 0x01);		//关了时钟中断......
//	timer_intr_init();
//	kbd_init();			//不知道为什么，没有绑定新的handler之前，老的handler无论按几次键盘都只输出一次......

	switch_to_user_mode();

//	asm volatile ("int $0x03;");

	printf("haha\n");

	while(1);
}
