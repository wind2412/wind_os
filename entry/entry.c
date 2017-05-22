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
#include <debug.h>


void init()
{
	clear_screen();
	putc('h');
	putc('e');
	putc('l');
	putc('l');
	putc('o');
	putc('\n');
	
	init_elf_tables();		//开启debug和backtrace

	gdt_init();
	idt_init();
	pic_init();
	tss_init();
	outb(0x21, 0x01);		//关了时钟中断......
//	timer_intr_init();
	kbd_init();


	switch_to_user_mode();
	switch_to_kern_mode();

//	asm volatile ("int $0x3;");

	print_backtrace();		//打印堆栈～debug成功。

	while(1);
}
