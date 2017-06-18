/*
 * proc.h
 *
 *  Created on: 2017年6月8日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_PROC_H_
#define INCLUDE_PROC_H_

#include <vmm.h>
#include <pic.h>
#include <idt.h>
#include <tss.h>

#define MAX_PROCESS 		4096
#define MAX_PID				8192
#define KTHREAD_STACK_PAGE	2

enum proc_lifecircle{
	TASK_READY = 0,
	TASK_SLEEPING = 1,
	TASK_RUNNABLE = 2,
	TASK_ZOMBIE = 3,
};

enum proc_waitstate{
	NOT_WT	 = 0,
	WT_CHILD = 1,
};

struct context{
	u32 eip;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esi;
	u32 edi;
	u32 ebp;
	u32 esp;
};

/**
 * 进程控制块PCB
 */
struct pcb_t{
	enum proc_lifecircle state;		//进程的状态
	int pid;
	u32 start_stack;
	struct pcb_t *parent;
	struct mm_struct *mm;
	struct pde_t *backup_pde;
	struct context context;
	struct idtframe *frame;
	enum proc_waitstate waitstate;
	struct pcb_t *cptr, *yptr, *optr;
	struct list_node node;
	struct list_node semaphore_test_node;	//设计这个是因为pcb_t中的node是不能使用的，因为原先已经有用了......因此必须设计一个别的数据结构保存node，用来semaphore的索引。
};

/****************中断的处理(atomic)*************/
u32 read_eflags();
void atom_disable_intr();
void atom_enable_intr();

/***************位图的pid处理*******************/
inline __attribute__((always_inline)) void set_bit(u32 *num, int pos, int is_set_one){		//把一个32bit的u32整数某位置位为0/1  	//注意这个pos是从右=>左，从0开始的。
	if(is_set_one)	(*num) |= (1 << pos);
	else			(*num) &= ~(1 << pos);
}

inline __attribute__((always_inline)) int get_bit(u32 *num, int pos){
	return ((*num) & (1 << pos)) == 0 ? 0 : 1;
};

int get_pid();		//得到一个新的pid

/*******************进程相关********************/

extern struct pcb_t *idle, *init_proc;	//内核线程0，1
extern struct pcb_t *current;		//当前进程
extern struct list_node proc_list;		//进程双向链表

//创建内核进程0和内核进程1
void proc_init();

struct pcb_t *create_pcb();

int default_proc_fn(void *what_you_want_to_say);	//用于kern_thread 1号initproc内核线程的fn

int fn_init_kern_thread(void *arg);		//init进程执行的函数。

int fn_sec_kern_thread(void *arg);		//进程2执行的函数

int kernel_thread (int (*fn)(void *), void *arg, u32 flags);

void kernel_thread_entry();

int do_fork(u32 flags, u32 stack, struct idtframe *);

int do_waitpid(int pid);

void do_exit();

void send_chld_to_init();

int set_kthread_stack(struct pcb_t *pcb, u32 offset);

int copy_mm(struct pcb_t *pcb, int is_share, struct mm_struct *copied_mm);

void delete_mm(struct mm_struct *mm);

void copy_thread(struct pcb_t *pcb, u32 stack, struct idtframe *);

void wakeup_process(struct pcb_t *pcb);

extern void switch_to(struct context *current, struct context *next);

extern void schedule();

void run_thread();

#endif /* INCLUDE_PROC_H_ */
