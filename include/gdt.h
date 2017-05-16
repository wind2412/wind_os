/*
 * gdt.h
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_GDT_H_
#define INCLUDE_GDT_H_

#include <types.h>

struct gdtdesc{
	u16 limit0_15;
	u16 base0_15;
	u8  base16_23;
	u8  gate_sign;		//41-45位
	u8  limit16_19:4;
	u8  other_sign:4;	//52-56位
	u8  base24_31;
} __attribute__((packed));

struct gdtr{
	u16 limit;
	u32 base;
} __attribute__((packed));

/**
 * gdt表。一共有6个，分别对应NULL段，内核代码段，内核数据段，用户代码段，用户数据段，TSS任务段(实现用户态和内核态切换)
 */
struct gdtdesc gdt[6];

/**
 * 生成gdt
 */
void gdt_init();

/**
 * AT&T汇编：lgdt
 */
void lgdt(struct gdtr *ptr);

/**
 * 批量设置段描述符表
 */
void set_seg_gate_desc(u32 seg_num, u32 base, u32 limit, u8 gate_sign, u8 other_sign);

#endif /* INCLUDE_GDT_H_ */
