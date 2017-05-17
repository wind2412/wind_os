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
 * idt帧。
 *
 * CPU自动由中断号*8作为OFFSET，由IDTR寄存器找到IDT表中中断的位置（自动）
 * 之后又IDT表中的cs selector，外带GDTR，可以找到GDT表中存放ISR的段的位置。当然，其实正常来讲内核中断的设置都是0x08段，也就是内核代码段。
 * 由IDT表中的OFFSET(与上文OFFSET不同)，可以找到ISR的位置。
 *
 * 中断由 用户态 -> 内核态 之前，会先压入用户栈栈地址和栈指针值。（自动）
 * 之后要触发中断，cpu自动压入 用户态下的eflags, cs, eip来保护现场。（自动）
 * 而后，程序员设定中断的trigger，也就是ucore下的vector。这个trigger只是“中断信号”，只起到压入异常码errorCode和intr_no的作用。（手动）
 * 之后，对应于ucore的__alltraps，此时会程序员手动调用pushl压入各个基本寄存器。然后会压入各种段寄存器，ds,es,fs,gs,ss.但是由于他们全是一样的，因此压入一个my_ax就可以了。（手动）
 *
 * 所以，这时候要push一个esp，也就是把当前esp push进去。因为这时esp指向结构体的头部，哈哈。太过机智了这个手段。于是执行call，这个方法需要有个参数struct idtframe *，需要通过push保存，正好也就是esp。
 *
 * 执行完了trigger，然后执行完call=>也就是执行完了isr的handler。这样的话，完事之后，就一定会回收原先的东西。也就是先popl my_ax，然后mov到各种寄存器ds,es,fs,gs,ss中。
 * 而后，肯定是要popl的。这样的话就可以恢复原先用户栈的现场了。
 * 随后，esp要+8(字节，即64位)，这样，intr_NO和errorCode会被跳过了。当然，errorCode会在handler就处理的。
 * 最后，恢复ss和esp。
 */
struct idtframe{
	u32 my_eax;

	u32 edi;
	u32 esi;
	u32 ebp;
	u32 no_use_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;		//程序员调用pusha指令压入这8个。全以32bit形式压入。

	u32 intr_No;		//程序员自行压入。
	u32 errorCode;				//注意这里！！！这里有玄机啊。errorCode这东西，是如果产生了的话，**CPU自动压入**。ISR[8-14]就产生错误码。如果产生错误，CPU会自动压进去。
								//对于没有errorCode的剩余所有(其实可以自己制定), 程序员需要自行压入一个0来代替！～

	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;		//用户esp 最先被压入
	u32 ss;			//用户栈栈地址
};

/**
 * ISR handlers			Interrupt service routine
 */
void (*handlers[256])(struct idtframe *);

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

/**
 * ISR函数。地址保存在IDT的offset中。ISR压入errorCode和intr_No，然后会调用寄存器存档函数isr_push，
 * 然后isr_push会调用中间层的trap函数处理idtframe结构体（trap函数是中间层，使用统一接口来通过idtframe调用指定的handler）。trap会调用（用户）指定的isr_handler.
 *
 * 保存errorCode和intr_No. 实际上，中断描述符表中存放的offset指的是这个函数。也就是ISR。而不是ISR handler。
 * 注意概念上的差异。ISR handler是“被ISR调用的，且可以由用户来指定的”。而ISR是放在IDT表中的。
 *
 */

//由于这里我使用了call，所以call跳过去之后会先隐式push进去esp和ebp。所以在struct idtframe会有两个32位值的多余。。所以一定会出错。
//解决方法：在gcc内联汇编里边加上了个my_push:标签。然后直接jmp到那边。
#define ISR_BEGIN_NOERROR(NUM)\
void isr_begin_no_error_##NUM()\
{						\
	asm volatile (		\
		"pushl $0x00;"	\
		"pushl %0;"			\
		"jmp my_push;"	\
		::"r"(NUM)			\
	);						\
}

#define ISR_BEGIN_ERROR(NUM)\
void isr_begin_error_##NUM()\
{							\
	asm volatile (			\
		"pushl %0;"			\
		"jmp my_push;"	\
		::"r"(NUM)			\
	);						\
}

/**
 * 此函数由ISR函数调用。
 * 在中断信号发出之后，（手动压入错误码errorCode）以及intr_No之后会执行。即把所有寄存器统统备份。
 */
void isr_push();

/**
 * ISR中间层
 */
void trap(struct idtframe *);

/**
 * 伪handler
 */
void isr_handler(struct idtframe *);

/**
 * 用户设置handler的方法
 */
void set_handler(int, void (*)(struct idtframe *));

