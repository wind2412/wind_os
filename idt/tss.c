/*
 * tss.c
 *
 *  Created on: 2017年5月18日
 *      Author: zhengxiaolin
 */

#include <tss.h>

/**
 * ltr有些不同于lgdt和lidt。它的可见位只有16位，这16位加载“某一tss在gdt的偏移量”
 */
void ltr(u16 offset_in_gdt){
	asm volatile ("ltr (%0)" ::"r"(offset_in_gdt));
}

void tss_init()
{
	//在GDT表的第六项设置我们的第一个tss段：TSS Descriptor		//注意：因为GDT一开始就设置了6项，所以不用在这里lgdt。
	set_seg_gate_desc(5, (u32)&tss0, sizeof(tss0), 0xE9, 0x04);
	//放到TR寄存器里边，使用ltr命令
	ltr(5*8);		//offset_in_gdt <- 每个gdtdesc占用8字节64位，5号tss前边共有0,1,2,3,4,共计5个描述符表。因此5*8.

	//在IDT的第120项设置(任务门描述符)switch_to_user_mode	121项的switch_to_kern_mode并不用修改。因为DPL是0x00，和其他中断没有区别.
	extern u32 _intrs[];
	set_intr_gate_desc(120, /*(u32)&_intrs[i]*/ _intrs[120], 0x08, IGD, 0x03, 1);		//与IDT项的不同之处只在于DPL：0x00->0x03  而switch_to_kern_mode并不用修改。

	//设置IDT第120项和第121项的handler！也就是switch_to_user_mode和switch_to_kern_mode.
	handlers[120] = switch_to_user_mode;
	handlers[121] = switch_to_kern_mode;
}

void switch_to_user_mode()
{

}

void switch_to_kern_mode()
{

}
