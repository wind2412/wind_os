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
 * idt帧。
 * 中断由 用户态 -> 内核态 之前，会先压入用户栈栈地址和栈指针值。（自动）
 * 之后要触发中断，cpu自动压入 用户态下的eflags, cs, eip来保护现场。（自动）
 * 而后，程序员设定中断的trigger，也就是ucore下的vector。这个trigger只是“中断信号”，只起到压入异常码errorCode和intr_no的作用。（手动）
 * 之后，对应于ucore的__alltraps，此时会程序员手动调用pushl压入各个基本寄存器。然后会压入各种段寄存器，ds,es,fs,gs,ss.但是由于他们全是一样的，因此压入一个my_ax就可以了。（手动）
 *
 * 隐藏步骤：最后会由于要获取到intr_No来取得索引(*8)。其实在C代码根本不需要乘。因为固定占8字节，只要号码就可以访问8字节一个的数组了。
 * 所以，这时候要push一个esp，也就是把当前esp push进去。因为这时esp指向结构体的头部，哈哈。太过机智了这个手段。于是执行call，这个方法需要有个参数struct idtframe *，需要通过push保存，正好也就是esp。
 *
 * 执行完了trigger，然后执行完call=>也就是执行完了isr的handler。这样的话，完事之后，就一定会回收原先的东西。也就是先popl my_ax，然后mov到各种寄存器ds,es,fs,gs,ss中。
 * 而后，肯定是要popl的。这样的话就可以恢复原先用户栈的现场了。
 * 随后，esp要+8(字节，即64位)，这样，intr_NO和errorCode会被跳过了。当然，errorCode会在handler就处理的。
 * 最后，恢复ss和esp。
 */
struct idtframe{
	u32 my_ax;

	u32 edi;
	u32 esi;
	u32 ebp;
	u32 esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;		//程序员调用pusha指令压入这8个。全以32bit形式压入。

	u32 intr_No;		//程序员自行压入。
	u32 errorCode;

	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;		//用户esp 最先被压入
	u32 ss;			//用户栈栈地址
};

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
