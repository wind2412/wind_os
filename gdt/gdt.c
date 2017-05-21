/*
 * gdt.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <gdt.h>

struct gdtr my_gdtr;

void gdt_init()
{
	set_seg_gate_desc(0, 0, 0, 0, 0);
	set_seg_gate_desc(1, 0x00000000, 0xFFFFFFFF, 0x9A, 0x0C);		//内核代码段
	set_seg_gate_desc(2, 0x00000000, 0xFFFFFFFF, 0x92, 0x0C);		//内核数据段
	set_seg_gate_desc(3, 0x00000000, 0xFFFFFFFF, 0xFA, 0x0C);		//用户代码段
	set_seg_gate_desc(4, 0x00000000, 0xFFFFFFFF, 0xF2, 0x0C);		//用户数据段
//	set_seg_gate_desc(5, 0x00000000, 0xFFFFFFFF, 0x9A, 0x0C);

	my_gdtr.base = (u32)&gdt;
	my_gdtr.limit = sizeof(gdt) - 1;

	lgdt(&my_gdtr);

	asm volatile (				//刷新所有非CS的段选择子为[数据段]......为毛是数据段????????因为只有CS是代码段阿（
		"movw $0x10, %ax;"		//由于有了gdt[3]和gdt[4]两个用户段，那么在这里设置数据段的时候，要把ds,es,ss设为内核数据段，把fs,gs设为用户数据段。
		"movw %ax, %ds;"
		"movw %ax, %es;"
		"movw %ax, %ss;"
		"movw $0x23, %ax;"		//0x08|ring0(0x08->8)是内核代码段。0x10|ring0(0x10->16)是内核数据段。
		"movw %ax, %fs;"		//0x18|ring3(0x1B->27)是用户代码段。0x20|ring3(0x23->35)是用户数据段。。。此刻我是懵逼的。不知道为毛要或上ring级...不过这东西不能强求。能理解就理解，不理解先过。??????
								//原来如此理解了。因为段寄存器，即段选择子的最后两位应该是CS.RPL的。而RPL是可以复用的，也就是，整个CS代表了代码段的位置(会把最后两位RPL忽略掉，这样可以节省位数)。而RPL不忽略的时候，
								//这时RPL才代表访问权限。～～
		"movw %ax, %gs;"
		"ljmp $0x08, $1f;"		//这句是为了刷新cs段选择子的。设成了内核代码段，因为ljmp会有两个参数，因此第二个参数的意思是刷新cs + 向前跳转到1:的位置。???????其实毫无卵用，就是凑个参数而已。因为ljmp有刷新CS的作用，然而同时又需要两个参数。
		"1:;"
	);
}

void lgdt(struct gdtr *ptr)
{
	asm volatile ("lgdt (%0);" :: "r"(ptr));
}

void set_seg_gate_desc(u32 seg_num, u32 base, u32 limit, u8 gate_sign, u8 other_sign)
{
	gdt[seg_num].base0_15 = base;
	gdt[seg_num].base16_23 = base >> 16;
	gdt[seg_num].base24_31 = base >> 24;
	gdt[seg_num].gate_sign = gate_sign;
	gdt[seg_num].limit0_15 = limit;
	gdt[seg_num].limit16_19 = limit >> 16;
	gdt[seg_num].other_sign = other_sign;
}
