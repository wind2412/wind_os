/*
 * idt.h
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <types.h>

#define IGD	0x1		//中断门描述符的话
#define TGD 0x2		//陷阱门描述符的话


/**
 * 中断描述符结构体
 */
struct idtdesc{
	u16 offset0_15;
	u16 selector;
	u8  no_use;
	u8  gate_sign:5;
	u8  dpl:2;
	u8  p:1;
	u16 offset16_32;
} __attribute__((packed));

/**
 * idtr寄存器。
 */
struct idtr{
	u16 limit;
	u32 base;
} __attribute((packed));

/**
 * 中断描述符表
 */
struct idtdesc idt[256];

/**
 * ISR handlers			Interrupt service routine
 */
u32 *handler[256];

/**
 * 生成idt
 */
void idt_init();

/**
 * AT&T汇编：lidt
 */
void lidt(struct idtr *ptr);

/**
 * 批量设置中断描述符表
 */
void set_intr_gate_desc(u32 interrupt, u32 offset, u16 selector, u8 gate_sign, u8 dpl, u8 p);
