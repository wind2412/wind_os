/*
 * idt.c
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#include <idt.h>
#include <stdio.h>

/**
 * 以下是256个ISR		错误的设计。因为这相当于函数，到时候会自动进行多余的push %esp操作......mdzz......
 */
//ISR_BEGIN_NOERROR(0)
//ISR_BEGIN_NOERROR(1)
//ISR_BEGIN_NOERROR(2)
//ISR_BEGIN_NOERROR(3)
//ISR_BEGIN_NOERROR(4)
//ISR_BEGIN_NOERROR(5)
//ISR_BEGIN_NOERROR(6)
//ISR_BEGIN_NOERROR(7)
//
//ISR_BEGIN_ERROR(8)
//ISR_BEGIN_ERROR(9)
//ISR_BEGIN_ERROR(10)
//ISR_BEGIN_ERROR(11)
//ISR_BEGIN_ERROR(12)
//ISR_BEGIN_ERROR(13)
//ISR_BEGIN_ERROR(14)
//
//ISR_BEGIN_NOERROR(15)
//ISR_BEGIN_NOERROR(16)
//ISR_BEGIN_NOERROR(17)
//ISR_BEGIN_NOERROR(18)
//ISR_BEGIN_NOERROR(19)
//ISR_BEGIN_NOERROR(20)
//ISR_BEGIN_NOERROR(21)
//ISR_BEGIN_NOERROR(22)
//ISR_BEGIN_NOERROR(23)
//ISR_BEGIN_NOERROR(24)
//ISR_BEGIN_NOERROR(25)
//ISR_BEGIN_NOERROR(26)
//ISR_BEGIN_NOERROR(27)
//ISR_BEGIN_NOERROR(28)
//ISR_BEGIN_NOERROR(29)
//ISR_BEGIN_NOERROR(30)
//ISR_BEGIN_NOERROR(31)
//ISR_BEGIN_NOERROR(32)
//ISR_BEGIN_NOERROR(33)
//ISR_BEGIN_NOERROR(34)
//ISR_BEGIN_NOERROR(35)
//ISR_BEGIN_NOERROR(36)
//ISR_BEGIN_NOERROR(37)
//ISR_BEGIN_NOERROR(38)
//ISR_BEGIN_NOERROR(39)
//ISR_BEGIN_NOERROR(40)
//ISR_BEGIN_NOERROR(41)
//ISR_BEGIN_NOERROR(42)
//ISR_BEGIN_NOERROR(43)
//ISR_BEGIN_NOERROR(44)
//ISR_BEGIN_NOERROR(45)
//ISR_BEGIN_NOERROR(46)
//ISR_BEGIN_NOERROR(47)
//ISR_BEGIN_NOERROR(48)


//由于ds,es,fs,gs,ss都是一样的。所以直接保存一个就得了。
//void isr_push()
//{
//	asm volatile (
//			"my_push:pusha;"				//详见idtframe结构体的说明。
//			"movw %ds, %ax;"
//			"pushl %eax;"
//
//			"movw $0x10, %ax;"
//			"movw %ax, %ds;"
//			"movw %ax, %es;"
//			"movw %ax, %fs;"
//			"movw %ax, %gs;"
//			"movw %ax, %ss;"
//
//			"pushl %esp;"
//
//			"call trap;"
//
//			"addl $0x04, %esp;"
//
//			"popl %eax;"
//
//			"movw %ax, %ds;"
//			"movw %ax, %es;"
//			"movw %ax, %fs;"
//			"movw %ax, %gs;"
//			"movw %ax, %ss;"
//
//			"popa;"
//
//			"addl $0x08, %esp;"
//
//			"iret"
//		);
//}

void trap(struct idtframe *frame){
//	printf("interrupt ");	//这个trap只是调用真正的handler的中间层。
//	((void (*)(struct idtframe *))handlers[frame->intr_No])(frame);		//调用handler
//	((void *)handlers[frame->intr_No](struct idtframe *))(frame);
	print_idtframe(frame);
	handlers[frame->intr_No](frame);
}

void print_idtframe(struct idtframe *frame)
{
	printf("***************************\n");
	printf("frame->my_eax: %x\n", frame->my_eax);
	printf("frame->edi: %x\n", frame->edi);
	printf("frame->esi: %x\n", frame->esi);
	printf("frame->oesp: %x\n", frame->no_use_esp);
	printf("frame->ebx: %x\n", frame->ebx);
	printf("frame->edx: %x\n", frame->edx);
	printf("frame->ecx: %x\n", frame->ecx);
	printf("frame->eax: %x\n", frame->eax);
	printf("frame->intr_No: %x\n", frame->intr_No);
	printf("frame->errorCode: %x\n", frame->errorCode);
	printf("frame->eip: %x\n", frame->eip);
	printf("frame->cs: %x\n", frame->cs);
	printf("frame->eflags: %x\n", frame->eflags);
	printf("frame->esp: %x\n", frame->esp);
	printf("frame->ss: %x\n", frame->ss);
	printf("***************************\n");
}

void isr_handler(struct idtframe *frame){		//真·用户定义handler
	if(frame->intr_No != 32){	//时钟中断太烦人了......
		printf("interrupt %d has been triggered!\n", frame->intr_No);
	}
}

void set_handler(int intr_No, void (* handler)(struct idtframe *)){
	handlers[intr_No] = handler;
}

/**
 * idtr一定要放在全局，能够搜索到
 */
struct idtr my_idtr;

/**
 * ISR 这个“例程”在这里变成IDT之后，CPU确实是会自动寻找的。而且能完美找到“例程”的位置。所谓“例程”,
 * 顾名思义，就是完全一样的程序。唯一不同的，就是每个例程中断信号不同，压入的intr_No不同，仅此而已。由这个例程，才会调用handler。但是用户想要
 * 指定handler，应该是没有接口的......这点怎么办？？原来如此，这里需要像herlex一样，设置数组，每次trap()查到intr_No之后，直接调用数组里的handler。
 * 然后给用户开放一个接口能够直接修改数组里的函数就好了！！！
 */
void idt_init(){
	//先设置所有的handler
	for(int i = 0; i < 256; i ++){
		set_handler(i, isr_handler);
	}

	extern u32 _intrs[];

	//再设置所有的IDT
	for(int i = 0; i < 256; i ++){
		set_intr_gate_desc(i, /*(u32)&_intrs[i]*/ _intrs[i], 0x08, IGD, 0x00, 1);	//由于汇编中，_intrs[i]内部本身就存放指针，因此不用再取指针了。
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

