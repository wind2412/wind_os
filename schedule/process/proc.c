/*
 * proc.c
 *
 *  Created on: 2017年6月8日
 *      Author: zhengxiaolin
 */

#include <proc.h>

struct pcb_t *idle, *init_proc;	//内核线程0，1

struct pcb_t *current;		//当前进程

struct list_node proc_list;		//进程双向链表

int proc_num;

/****************中断的处理(atomic)*************/
u32 read_eflags()
{
	u32 eflags;
	asm volatile ("pushfl; popl %0;":"=r"(eflags));
	return eflags;
}

void atom_disable_intr()
{
	if((read_eflags() & 0x200) != 0){		//此时eflags设定允许了中断。
		cli();		//关中断
	}
}
void atom_enable_intr()
{
	sti();
}

/***************位图的pid处理*******************/
u32 pids[128];		//位图。

int get_pid()		//得到一个新的pid
{
	int bitmap_num = sizeof(pids)/sizeof(u32);		//实际上就是128
	for(int i = 0; i < bitmap_num; i ++){
		if(pids[i] != 0xFFFFFFFF){
			for(int j = 0; j < 32; j ++){
				if(get_bit(&pids[i], 31-j) == 0){		//如果从左->右某一位为0，即中标啦～
					set_bit(&pids[i], 31-j, 1);
					return i*32+j;
				}
			}
		}
	}
	return -1;		//全满，get_pid()失败。
}

/*******************进程相关********************/
struct pcb_t *create_pcb()
{
	struct pcb_t *pcb = (struct pcb_t *)malloc(sizeof(struct pcb_t));
	extern struct pde_t *pd;
	if(pcb == NULL)	return NULL;

	pcb->state = TASK_READY;
	pcb->pid = -1;
	pcb->start_stack = 0;
	pcb->flags = 0;
	pcb->mm = NULL;
	memset(&pcb->context, 0, sizeof(struct context));
	pcb->backup_pde = pd;
	pcb->parent = NULL;
	//node没有初始化
	return pcb;
}

int default_proc_fn(void *what_you_want_to_say)
{
	printf("hello, proc No.1! %s.\n", (const char *)what_you_want_to_say);
	return 0;		//errorCode. 由于fn返回之后，会把return值装载在eax中，因此可以充当errorCode。
}

void proc_init()
{
	list_init(&proc_list);	//初始化proc_list链表
	memset(pids, 0, sizeof(pids));

	if((idle = create_pcb()) == NULL){
		panic("can't init_proc proc 0... wrong.\n");
	}

	idle->pid = get_pid();		//这个pid一定是0.
	extern u8 kern_stack[];
	idle->start_stack = (u32)kern_stack + KTHREAD_STACK_PAGE * PAGE_SIZE;
	idle->state = TASK_READY;

	current = idle;

	proc_num ++;

	if((kernel_thread(default_proc_fn, "oh my god", 0)) == -1){
		panic("can't init_proc proc 1... wrong.\n");
	}

	init_proc = GET_OUTER_STRUCT_PTR(proc_list.next, struct pcb_t, node);

}

int kernel_thread(int (*fn)(void *), void *arg, u32 flags)
{
	struct idtframe frame;
	memset(&frame, 0, sizeof(frame));
	frame.cs = 0x08;						//设置idtframe。这个idtframe放在pcb->start_stack的最高地址处。里边有将要恢复的各种变量。之所以要利用中断，是因为后边系统调用需要中断，这里一起用了吧。
	frame.my_eax = frame.ss = 0x10;
	frame.ebx = (u32)fn;
	frame.edx = (u32)arg;
	frame.eip = (u32)kernel_thread_entry;	//在switch_to之后，eip会被设置为copy_thread内部pcb->context.eip。此eip设置为中断my_push的中间后部pop恢复寄存器部分。
											//届时iret时，eip会被恢复成frame.eip==>kernel_thread_entry.然后就会执行ebx，edx中的fn函数了。之后线程消亡。
	return do_fork(flags | 0x100, 0, &frame);		//????????
}

int do_fork(u32 flags, u32 stack, struct idtframe *frame)		//这个do_fork其实并不算fork。它的pcb全是通过额外传入的参数frame设置的。如果是fork，理当从current->frame设置吧。
{
	if(proc_num > MAX_PROCESS)	return -1;

	//创建pcb
	struct pcb_t *pcb = create_pcb();
	if(pcb == NULL)		return -1;

	//填充pcb
	if(set_kthread_stack(pcb) == -1){
		free(pcb);
		return -1;
	};
	pcb->pid = get_pid();		//这里应该关中断----> atom
	if(pcb->pid == -1){
		free(pcb);
		free_page(la_addr_to_pg(pcb->start_stack), KTHREAD_STACK_PAGE);
		return -1;
	}
	pcb->flags = flags;
	pcb->parent = current;
	list_insert_after(&proc_list, &pcb->node);		//插入node
	proc_num ++;
	if(copy_mm(flags, pcb) == -1){

	}
	copy_thread(pcb, stack, frame);

	wakeup_process(pcb);

	return pcb->pid;
}

void do_exit(int errorCode)
{
	panic("hahaha!!exit!!\n");
}

int copy_mm(){
	return -1;
}

void copy_thread(struct pcb_t *pcb, u32 stack, struct idtframe *frame)
{
	pcb->frame = (struct idtframe *)(pcb->start_stack - sizeof(struct idtframe));		//frame->esp是“利用中断后方恢复”时必要的frame指针位置。通过frame里边设置的寄存器值，进行恢复。
	//先把整个idtframe放到pcb->start_stack的最高地址处。然后设置。
	*(pcb->frame) = *frame;
	//设置frame->esp。虽然这个在内核态没啥大用。这个frame->esp会被用到“iret=>内核态切换到用户态 由iret恢复”。
	pcb->frame->esp = stack;

	pcb->context.esp = (u32)pcb->frame;		//注意传入的是frame的位置。但是要提取出来frame->myeax的话，需要解下引用～毕竟是指针，访问需要frame->myeax.汇编的话，如果把它放到%esp中，访问myeax需要是0(%esp).==> *(esp + 0)
	extern u32 context_to_intr;		//C语言extern一个.globl字段的话，会自动得到内部的data值。如果需要得到.globl字段的地址，还要取&.....http://blog.csdn.net/smstong/article/details/54405649
	pcb->context.eip = (u32)&context_to_intr;		//汇编函数。
}

void kernel_thread_entry()
{
	asm volatile("push %edx; call *%ebx; push %eax; call do_exit;");
}

int set_kthread_stack(struct pcb_t *pcb)
{
	struct Page *pages = alloc_page(KTHREAD_STACK_PAGE);		//分配内核线程栈空间
	if(pages == NULL)	return -1;

	pcb->start_stack = pg_to_addr_la(pages) + KTHREAD_STACK_PAGE * PAGE_SIZE;
	return 0;
}

void wakeup_process(struct pcb_t *pcb)
{
	pcb->state = TASK_RUNNABLE;
}

void run_thread(struct pcb_t *pcb)
{
	if(current == pcb)	return;

	extern struct tss tss0;

	struct pcb_t *prev = current;
	current = pcb;
	//1.设置tss的ring0的esp0为内核线程栈的最高地址
	tss0.esp0 = pcb->start_stack;
	//2.设置页目录表
	asm volatile ("movl %0, %%cr3"::"r"(pcb->backup_pde));
	//3.切换进程上下文
	switch_to(&prev->context, &current->context);
}
