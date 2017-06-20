/*
 * monitor.c
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#include <monitor.h>

void monitor_init(struct monitor_t *monitor, int cond_num)
{
	semaphore_init(&monitor->mutex, 1);		//初始化互斥量为1
	semaphore_init(&monitor->next,  0);		//初始化入口等待队列的信号量为0
	monitor->next_count = 0;				//入口等待队列的pcb数为0个
	monitor->cvs = (struct condition_t *)malloc(sizeof(struct condition_t) * cond_num);
	for(int i = 0; i < cond_num; i ++){
		semaphore_init(&monitor->cvs[i].x_sem, 0);		//初始化资源等待队列的信号量为0
		monitor->cvs[i].x_count = 0;				//资源等待队列的pcb数为0个
		monitor->cvs[i].monitor = monitor;
	}
}

/**
 * 语义：请求资源得不到满足的时候会调用wait()函数，
 * 这时：current会被挂起，cond->x_count++，
 * 	 //// 本来这里应该让current沉睡并且调度，但是下一个要被唤醒的进程还没有准备好。因此暂时不调用P()。放到V()之后再schedule()。
 * 		然后，优先唤醒“入口紧急等待队列”当中的进程。
 * 		当monitor->next_count > 0 说明有在急等的进程还在等着进入管程，于是唤醒一个(通过V(&monitor->next))。但是这样的话，V操作只是把pcb->state变成TASK_RUNNING。但是并没有schedule()。因此这里需要我们把P()放到后边调用。
 * 		当monitor->next_count<= 0 说明没有急等的进程。那么这时我们可以释放mutex。
 * 		并把current加入到cond->x_sem的wait_queue中(通过P(&cond->x_sem))。这样，管程便被空出来了。schedule()会调用急等next信号中的pcb。
 */
//struct condition_t 内部的wait函数		(由于P()、V()操作已经避免中断了。管程内部不需要再禁止中断了。)
//即：把current挂起到资源等待队列，再从急等队列中唤醒一个。如果急等队列没有，那就从入口队列mutex唤醒一个。
void wait(struct condition_t *cond)
{
	cond->x_count ++;		//current将会因为资源不足而放置到cond->wait_queue中。因而数目+1.
	if(cond->monitor->next_count > 0){		//急等队列上边有，就唤醒急等队列中第一个pcb进入管程
		V(&cond->monitor->next);
	}else{									//否则放一个mutex的入口队列中的pcb进入管程
		V(&cond->monitor->mutex);
	}
	P(&cond->x_sem);		//沉睡current并且schedule()。因为schedule()在P()中，因此这句话必须放在后边，而不能放在cond->x_count++的直接后边，而是必须要等待下一个进入管程的进程被设置好唤醒pcb->state。即V()执行之后。
	cond->x_count --;		//当被condition::signal()由于资源足够而唤醒之后，再次被schedule()之后会切换到此进程，从这句醒来。所以不需要再等待了。从队列中删除即可。
	print_thread_chains();		//delete.
}

/**
 * 语义：由于请求资源足够，现在要调用signal()来唤醒一个cond->x_sem中wait_queue中的pcb。
 * 这时：需要检查此cond的wait_queue是否是空的。如果是空的，那么就空操作好了。
 * 		之后需要唤醒cond上wait_queue中的一个pcb，
 * 		然后，要把current强制抢占，放到next急等队列中去。而且，monitor->next_count ++.(还要执行P())，当然，这时会被schedule()。于是进程会被切换。
 * 		当然，当P()返回的时候，必然是当前刚刚被抢占的current进程被cond::wait()唤醒了。因而，monitor->next_count--.把current从next急等队列中删除。
 */
//struct condition_t 内部的signal函数
//即：把资源等待队列中的一个pcb唤醒，再把current抢占并挂起到急等队列。
void signal(struct condition_t *cond)
{
	if(cond->x_count > 0){
		cond->monitor->next_count ++;		//将要把current抢占并塞进急等队列中去了。
		V(&cond->x_sem);		//由于资源足够，唤醒cond的wait_queue中的一个pcb。
		P(&cond->monitor->next);		//抢占current并等待。
		cond->monitor->next_count --;	//当P()返回的时候，说明current在急等队列中被cond::wait()唤醒了。(此处是有进程切换的)，因此monitor->next_count--.
	}else{
				//空操作
	}
}

/**
 * 语义：进入管程
 */
//struct monitor 内部的enter函数
void enter(struct monitor_t *monitor)
{
	P(&monitor->mutex);		//进入管程的时候，先抓取互斥锁！
}

/**
 * 语义：退出管程
 */
//struct monitor 内部的leave函数
void leave(struct monitor_t *monitor)
{
	//代码和cond::wait()中的一部分差不多。
	if(monitor->next_count > 0){	//即将退出monitor管程的时候，检查是否急等队列里边还有pcb
		V(&monitor->next);			//有的话，激活pcb
	}else{
		V(&monitor->mutex);			//没有的话，释放mutex。让入口队列的pcb进入管程。
	}
}
