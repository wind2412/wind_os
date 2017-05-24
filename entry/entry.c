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
#include <pmm.h>

//见ucore指导书：0～640KB是一开始空闲的，0~4kb全都当做页目录表，5~8,9~12,13~16,17~20kb当做页表（临时）（Linux0.11实现）
__attribute__((section(".init.data"))) struct pde_t *pd  = (struct pde_t *)0x0000;
__attribute__((section(".init.data"))) struct pte_t *fst = (struct pte_t *)0x1000;
__attribute__((section(".init.data"))) struct pte_t *snd = (struct pte_t *)0x2000;

/**
 * 虚拟内存：程序员输入的内存都是0xC0100000+，但是实际上内核被加载到了0x100000上了。
 */

void init();

//模仿hx_kernel，建立临时的页表。ucore在我观察来看，好像并不太像“虚存”啊。因为VMA和LMA竟然是一样的。
__attribute__((section(".init.text"))) void kern_init()
{
	extern u8 kern_start[];
	//组建页目录表
	pd[0].pt_addr = (u32)snd >> 12;
	pd[0].os = 0;
	pd[0].sign = 0x3;
	pd[(u32)kern_start >> 22].pt_addr = (u32)snd >> 12;		//把0xC0000000的映射到0x2000上去。(自己指定目录表项，不必顺序映射)
	pd[(u32)kern_start >> 22].os = 0;
	pd[(u32)kern_start >> 22].sign = 0x3;				//最低的两位：R/W位和P位全都置一。

	//组建页表
	for(int i = 0; i < PAGE_SIZE/4; i ++){
		snd[i].page_addr = i;			//这个是计算好的。
		snd[i].os = 0;
		snd[i].sign = 0x3;
	}

	//设置页表
	asm volatile ("movl %0, %%cr3"::"r"(pd));

	//开启分页
	asm volatile ("movl %%cr0, %%eax; orl $0x80000000, %%eax; movl %%eax, %%cr0":::"%eax");

	//调用正常的初始化函数
	init();
}

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

	init_pmm();

	while(1);
}
