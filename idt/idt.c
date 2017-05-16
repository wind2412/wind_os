/*
 * idt.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <idt.h>
#include <stdio.h>

#define ISR_BEGIN()\
{\
	asm volatile (\
			"pushl %eax;"			\
			"pushl %ebx;"			\
			"pushl %ecx;"			\
			"pushl %edx;"			\
			"pushl %esp;"			\
			"pushl %ebp;"			\
			"pushl %esi;"			\
			"pushl %edi;"			\
			"pushl %ds;"			\
			"pushl %es;"			\
			"pushl %fs;"			\
			"pushl %gs;"			\
		);\
}

#define ISR_RET()\
{\
	asm volatile (\
			"popl %gs;"				\
			"popl %fs;"				\
			"popl %es;"				\
			"popl %ds;"				\
			"popl %edi;"			\
			"popl %esi;"			\
			"popl %ebp;"			\
			"popl %esp;"			\
			"popl %edx;"			\
			"popl %ecx;"			\
			"popl %ebx;"			\
			"popl %eax;"			\
		);\
}

void isr_template(){
	ISR_BEGIN();
	printf("this is the interrupt .\n");
	ISR_RET();
}

//#define ISR_(NUM)	ISR_##NUM

/**
 * idtr一定要放在全局，能够搜索到
 */
struct idtr my_idtr;

void idt_init(){
	for(int i = 0; i < 256; i ++){
		set_intr_gate_desc(i, (u32)&isr_template, 0x08, IGD, 0x00, 1);
	}
	my_idtr.base = (u32)&idt;
	my_idtr.limit = sizeof(idt) - 1;
	lidt(&my_idtr);
}

void lidt(struct idtr *ptr){
	asm volatile (
		"lidt (%0)"
		:
		:"r"(ptr)
	);
}

void set_intr_gate_desc(u32 interrupt, u32 offset, u16 selector, u8 gate_sign, u8 dpl, u8 p){
	idt[interrupt].offset0_15 = offset;
	idt[interrupt].offset16_32 = offset >> 16;
	idt[interrupt].selector = selector;
	if(gate_sign == IGD){
		idt[interrupt].gate_sign = 0xE;		//如果是中断门
	}else if(gate_sign == TGD){
		idt[interrupt].gate_sign = 0xF;		//如果是陷阱门
	}
	idt[interrupt].dpl = dpl;
	idt[interrupt].p = p;
	idt[interrupt].no_use = 0;
}

