/*
 * semaphore.c
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#include <semaphore.h>

void semaphore_init(struct semaphore_t *s, int value)
{
	s->value = value;
	list_init(&s->queue);
}

void P(struct semaphore_t *s)
{
	atom_disable_intr();
	s->value --;
	if(s->value < 0){			//如果s->value < 0的话  把current加入到wait_queue中			//也就是说，按照生产者--消费者，只有小于0的话，消费者阻塞。
		list_insert_before(&s->queue, &current->semaphore_test_node);
		//我们这里没有block，因此只能使用schedule来进行调度，其实也是一样。
		current->state = TASK_SLEEPING;		//只有通过V()才能继续调度
		schedule();
	}else{
								//空操作
	}
	atom_enable_intr();				//调度之前需要关闭中断！！...当我没说  结果好像也是一样的
}

void V(struct semaphore_t *s)
{
	atom_disable_intr();
	s->value ++;				//这里已经把s->value++了。原先P操作的时候，s->value的值<0的时候，会有进程休眠。这时+了1，因此变成了<=0的时候，需要唤醒一个刚才休眠的进程。
	if(s->value <= 0){			//如果s->value <= 0的话，唤醒一个进程						//也就是说，按照生产者——消费者，当≤0的时候，唤醒一个进程。
		struct pcb_t *pcb = GET_OUTER_STRUCT_PTR(s->queue.next, struct pcb_t, semaphore_test_node);
		pcb->state = TASK_RUNNABLE;			//唤醒进程。
		if(s->queue.next != &s->queue)	list_delete(s->queue.next);		//在wait_queue中删除此pcb
	}else{
								//空操作。
	}
	atom_enable_intr();
}
