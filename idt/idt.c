/*
 * idt.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <idt.h>
#include <stdio.h>

//由于ds,es,fs,gs,ss都是一样的。所以直接保存一个就得了。
#define ISR_BEGIN()\
{\
	asm volatile (\
			"pusha;"\
			"movw %ds, %ax;"\
			"pushl %ax;"\
						\
			"movw $0x10, %ax;"\
			"movw %ax, %ds;"\
			"movw %ax, %es;"\
			"movw %ax, %fs;"\
			"movw %ax, %gs;"\
			"movw %ax, %ss;"\
						\
			"pushl %esp;"\
						\
			"call trap;"\
						\
			"addl $0x04, %esp;"\
						\
			"popl %eax;"\
						\
			"movw $0x10, %ax;"\
			"movw %ax, %ds;"\
			"movw %ax, %es;"\
			"movw %ax, %fs;"\
			"movw %ax, %gs;"\
			"movw %ax, %ss;"\
						\
			"popa;"\
					\
			"addl $0x08, %esp;"\
					\
			"iret"\
		);\
}

void trap(struct idtframe *frame){
	printf("interrupt ");	//这个trap只是调用真正的handler的中间层。
	isr_handler(frame);
}

void isr_handler(struct idtframe *frame){
	printf("%d has been triggered!\n", frame->intr_No);
}

//#define ISR_(NUM)	ISR_##NUM

/**
 * idtr一定要放在全局，能够搜索到
 */
struct idtr my_idtr;

void idt_init(){
	for(int i = 0; i < 256; i ++){
		set_intr_gate_desc(i, (u32)&isr_handler, 0x08, IGD, 0x00, 1);	//ISR 这个“例程”在这里变成IDT之后，CPU确实是会自动寻找的。而且能完美找到“例程”的位置。所谓“例程”，
								//顾名思义，就是完全一样的程序。唯一不同的，就是每个例程中断信号不同，压入的intr_No不同，仅此而已。由这个例程，才会调用handler。但是用户想要
								//指定handler，应该是没有接口的......这点怎么办？？原来如此，这里需要像herlex一样，设置数组，每次trap()查到intr_No之后，直接调用数组里的handler。
								//然后给用户开放一个接口能够直接修改数组里的函数就好了！！！
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

