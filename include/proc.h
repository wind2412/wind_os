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

#define MAX_PROCESS 4096
#define MAX_PID		8192

enum proc_lifecircle{
	TASK_READY = 0,
	TASK_SLEEPING = 1,
	TASK_RUNNABLE = 2,
	TASK_ZOMBIE = 3,
};

struct context{
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esi;
	u32 edi;
	u32 ebp;
	u32 esp;
	u32 eip;
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
	u32 flags;
	struct list_node node;
};

/****************中断的处理(atomic)*************/
u32 read_eflags();
void atom_disable_intr();
void atom_enable_intr();

/***************位图的pid处理*******************/
inline __attribute__((always_inline)) void set_bit(u32 *num, int pos, int is_set_one);		//把一个32bit的u32整数某位置位为0/1
inline __attribute__((always_inline)) int get_bit(u32 *num, int pos);
int get_pid();		//得到一个新的pid

/*******************进程相关********************/

//创建内核进程0和内核进程1
void proc_init();

struct pcb_t *create_pcb();


void do_fork(struct pcb_t *parent);


#endif /* INCLUDE_PROC_H_ */
