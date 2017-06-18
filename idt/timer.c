/*
 * timer.c
 *
 *  Created on: 2017年5月17日
 *      Author: zhengxiaolin
 */

#include <timer.h>
#include <sched.h>

u32 jiffies = 0;

void timer_intr_init()
{
	outb(0x43, 0x34);									//开启时钟中断
	outb(0x40, (1193182/26) & 0xFF);					//时钟中断的输入频率是1193180，每秒中断200次
	outb(0x40, ((1193182/26)>>8) & 0xFF);				//分两次低字节高字节写入0x40 I/O端口

//	handlers[32] = timer_handler;		//注册一个time_handler函数。
	set_handler(32, timer_handler);
}

void timer_handler(struct idtframe *frame)
{
	jiffies ++;
//	printf("now it is %d intr.\n", jiffies);
	schedule();		//切换进程
}
