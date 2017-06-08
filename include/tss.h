/*
 * tss.h
 *
 *  Created on: 2017年5月18日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_TSS_H_
#define INCLUDE_TSS_H_

#include <types.h>
#include <stdio.h>
#include <gdt.h>
#include <idt.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif


struct tss{
	u16 back_link;	u16 bzero;			//back_link由CPU在切换CPL的时候自动压入。

	u32 esp0;
	u16 ss0;		u16 bzero0;

	u32 esp1;
	u16 ss1;		u16 bzero1;

	u32 esp2;
	u16 ss2;		u16 bzero2;

	u32 cr3_PDBR;
	u32 eip;
	u32 eflags;

	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;

	u16 es;			u16 bzero3;
	u16 cs;			u16 bzero4;
	u16 ss;			u16 bzero5;
	u16 ds;			u16 bzero6;
	u16 fs;			u16 bzero7;
	u16 gs;			u16 bzero8;

	u16 ldt;		u16 bzero9;

	u16 debug_trap_bit:1;		u16 bzero10:7;

	u16 io_map_base;

} __attribute__((packed));

//第一个tss段
struct tss tss0;

//初始化tss在GDT的tss表
void tss_init();

//IDT No.120和No.121的handler
void switch_to_user_handler(struct idtframe *frame);
void switch_to_kern_handler(struct idtframe *frame);

//切换到用户模式
void switch_to_user_mode();

//切换到内核模式
void switch_to_kern_mode();



#endif /* INCLUDE_TSS_H_ */
