/*
 * proc.c
 *
 *  Created on: 2017年6月8日
 *      Author: zhengxiaolin
 */

#include <proc.h>

struct pcb_t *zero, *init;	//内核线程0，1

struct pcb_t *cur;		//当前进程

struct list_node proc_list;

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

void set_bit(u32 *num, int pos, int is_set_one)	//注意这个pos是从右=>左，从0开始的。
{
	if(is_set_one)	(*num) |= (1 << pos);
	else			(*num) &= ~(1 << pos);
}

int get_bit(u32 *num, int pos)
{
	return ((*num) & (1 << pos)) == 0 ? 0 : 1;
}

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

void proc_init()
{
	list_init(&proc_list);	//初始化proc_list链表
	memset(pids, 0, sizeof(pids));
	set_bit(&pids[0], 31, 0);		//设置0号进程已经占用。即最高位第31位已经为1了。

	if((zero = create_pcb()) == NULL){
		panic("can't init proc 0... wrong.\n");
	}


}
