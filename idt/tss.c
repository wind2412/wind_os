/*
 * tss.c
 *
 *  Created on: 2017年5月18日
 *      Author: zhengxiaolin
 */

#include <tss.h>

/**
 * ltr有些不同于lgdt和lidt。它的可见位只有16位，这16位加载“某一tss在gdt的偏移量”
 */
void ltr(u16 offset_in_gdt){
	asm volatile ("ltr %0;" ::"r"(offset_in_gdt));		//注意这里"ltr %0"不能是"ltr (%0)"
}

//设置tss的ss0内核栈
u8 kern_stack[1024];

void tss_init()
{
	//先把内核栈设置到第一个tss结构体中去
	tss0.back_link = 0;
	tss0.ss0  = 0x08;		//设置内核栈段选择子
	tss0.esp0 = (u32)kern_stack + sizeof(kern_stack);	//tss0.esp0指向内核栈底部，而后即使切换，也一直不会变了。

	//在GDT表的第六项设置我们的第一个tss段：TSS Descriptor		//注意：因为GDT一开始就设置了6项，所以不用在这里lgdt。
	//之前tss这里的粒度位计算成了0x89......然后各种启动不了......简直要死.......后来改成0x8B之后终于变成不断0x0d中断，感觉好幸福啊TAT
	//0x89的话，4位的type位就是二进制1001(是9)，而0x8B的话，4位的type位就是二进制的1011(B)。只有一位的不同，但是会导致不同的结果！
	//不同的位为type位的第一位(0开始计数)，第一位表示可读还是可执行，如果是0，就是只可执行不可读，如果是1就是可读。一开始设置0是不可读,,,当然会各种错误了......TAT
	set_seg_gate_desc(5, (u32)&tss0, sizeof(tss0), 0x8B, 0x04);		//TSS的DPL也设置成0x00的ring0！如果设置成user会怎样？？？？？？？？就会啥情况下都可以切换了吗/？？？？？
	//放到TR寄存器里边，使用ltr命令
	ltr(5*8);		//offset_in_gdt <- 每个gdtdesc占用8字节64位，5号tss前边共有0,1,2,3,4,共计5个描述符表。因此5*8.

	//在IDT的第121项设置(任务门描述符)切换到用内核。120项的切换到内核并不用修改。因为DPL是0x00，和其他中断没有区别.
	extern u32 _intrs[];
	set_intr_gate_desc(121, /*(u32)&_intrs[i]*/ _intrs[121], 0x08, IGD, 0x03, 1);		//与IDT项的不同之处只在于DPL：0x00->0x03  而IDT No.120并不用修改。

	//设置IDT第120项和第121项的handler！也就是switch_to_user_handler和switch_to_kern_handler.
	handlers[120] = switch_to_user_handler;
	handlers[121] = switch_to_kern_handler;
}

//这里非常难理解啊。详见https://wenku.baidu.com/view/a7edfdc233687e21ae45a922.html?re=view吧。这个写的简直不能再棒o(*////▽////*)q
//首先。因为从ring0->ring3，int $120之后也是不会切换的。因为无论是之前，还是中断发生，都在ring0啊。所以，ring0->ring3必须通过iret来实现切换。因此，这个[函数本身]还是在内核态的！切记切记！
//不打算按照ucore的实现了。虽然那一步跳栈实在是不能再漂亮了。
void switch_to_user_handler(struct idtframe *frame)
{
//	printf("%d\n", tss0.back_link);
//	printf("%d\n", tss0.ss0);
//	printf("%d\n", tss0.esp0);
//	printf("%d\n", tss0.esp1);
//	printf("%d\n", tss0.esp2);
	frame->cs = 0x18|0x03;		//设置iret返回的cs是用户的cs
	frame->my_eax = frame->ss = 0x20|0x03;		//设置iret返回的ds，es，fs，gs和ss。
	frame->eflags |= 0x3000;					//防止用户无法使用外设中断IO，因此更改了eflags IO的权限，即eflags的IOPL双位设置为0x11，即用户权限。注意内核切换回来要改回去。
	frame->esp = (u32)&frame->esp;					//按照上边给出的链接，设置esp为&frame->esp。因为由报告书的图可知，iret要设置的esp实际上由于当时int调用并没有改变CPL，因此并没有push ss和esp进去。
												//而新的用户栈按图来看，设置的位置恰好是老栈的&esp位置。因此做了如下改动。即，把用户栈的位置设成了&frame->esp～0的空间。因为已经全部iret完了，frame已经没有用了。
												//因此可以就地设栈了。然而内核栈好像还没有过去？？内核栈到底是哪里，这个一直是一个问题啊。
}

//https://chyyuu.gitbooks.io/simple_os_book/zh/chapter-2/hardware_intr.html
//详参第四段4.，这里说的简直非常标准。
//如果int之前在ring3，就会触发tr和tss。否则的话，第4.步是不做的。
//实际上，只要用户态ring3调用int中断(不一定非要是switch_to_kern_mode)，就一定会触发tr和tss，然后瞬间切换到内核栈kern_stack[1024].然后CPU会在中断的起始之前压入ring3的ss和esp。
void switch_to_kern_handler(struct idtframe *frame)
{
//	printf("haha");
}

void switch_to_user_mode()
{
	asm volatile ("sub $0x8, %esp;");		//这里是局部变量！！！因为kern->user并不压入ss和esp！！所以在int之后的switch_to_user_handler中，要自行进行压入操作。
	asm volatile ("int $120;");
	asm volatile ("add $0x8, %esp;");		//清除局部变量。等同于ucore中的 "mov %ebp, %esp".
}

void switch_to_kern_mode()
{
	asm volatile ("int $121;");				//这里因为进行了CPL ring3->ring0的转换，因此CPU检测到之后，会自行压入ss3和esp3在frame和tss中，并从tss取出ss0和esp0(这个其实就是0，因为内核默认空栈。)
}
