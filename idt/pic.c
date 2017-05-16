/*
 * pic.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <pic.h>

/**
 * 初始化8259A芯片
 */
void pic_init()
{
	outb(0x21, 0xFF);		//填上mask
	outb(0xA1, 0xFF);

	outb(0x20, 0x11);		//ICW1
	outb(0x21, 32);			//ICW2
	outb(0x21, 0x04);		//ICW3
	outb(0x21, 0x03);		//ICW4

	outb(0xA0, 0x11);
	outb(0xA1, 32+8);
	outb(0xA1, 2);			//选定软连接主片2号接口
	outb(0xA1, 0x03);

	outb(0x21, 0x00);		//解除mask
	outb(0xA1, 0x00);

//	sti();		//....不可以在这里打开中断。
}

void sti()
{
	asm volatile ("sti");
}
