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
	set_seg_gate_desc(1, 0x00000000, 0xFFFFFFFF, 0x9A, 0x0C);
	set_seg_gate_desc(2, 0x00000000, 0xFFFFFFFF, 0x92, 0x0C);
	set_seg_gate_desc(3, 0x00000000, 0xFFFFFFFF, 0xFA, 0x0C);
	set_seg_gate_desc(4, 0x00000000, 0xFFFFFFFF, 0xF2, 0x0C);
	set_seg_gate_desc(1, 0x00000000, 0xFFFFFFFF, 0x9A, 0x0C);

	my_gdtr.base = (u32)&gdt;
	my_gdtr.limit = sizeof(gdt) - 1;

	lgdt(&my_gdtr);
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
