/*
 * sched.c
 *
 *  Created on: 2017年6月8日
 *      Author: zhengxiaolin
 */

#include <sched.h>


void schedule()
{
	struct list_node *begin, *next;
	begin = (current == idle) ? &proc_list: &current->node;
	next = begin->next;
	struct pcb_t *target_pcb = NULL;
	while(next != begin){		//因为一开始proc_list就肯定有一个init线程，因此不必担心是空的进程。
		if(next == &proc_list)	continue;	//如果回到开头，必须跳过。因为不是pcb结构体。
		target_pcb = GET_OUTER_STRUCT_PTR(next, struct pcb_t, node);
		if(target_pcb->state == TASK_RUNNABLE){
			break;	//已经找到
		}
		next = next->next;
	}
	//循环退出有两种情况：1.一个符合TASK_RUNNABLE都没有找到而退出 2.找到而退出
	if(target_pcb != NULL && target_pcb->state == TASK_RUNNABLE){
		run_thread(target_pcb);
	}else{
		run_thread(idle);
	}
}
