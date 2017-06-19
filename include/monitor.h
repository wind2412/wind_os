/*
 * monitor.h
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#ifndef SCHEDULE_SYNCHRONIZE_MONITOR_H_
#define SCHEDULE_SYNCHRONIZE_MONITOR_H_

#include <semaphore.h>

/**
 * Extra Reference：
 * https://wenku.baidu.com/view/3cf2e85c195f312b3069a57d.html	管程的概念及实现	这个非常好！！！
 * https://www.zhihu.com/question/30641734						《应该如何理解管程》
 * http://blog.forec.cn/2016/11/24/os-concepts-6/				管程的实现
 */

/**
 * 注意：管程内部的信号量与正常的信号量不同的地方在于：monitor内部及condition的所有semaphore都是初始化semaphore->value==0的。
 * 		而正常的semaphore是可以初始化为 > 0 的。
 * 		因此，semaphore.P()正式变成了“沉睡current进程”，semaphore.V()正式变成了“唤醒此semaphore内部wait_queue的第一个进程”。
 * 		(原来在生产者消费者那时，还有semaphore full && empty。每次执行P()和V()的时候，如果值过大的话，一般都是直接-1/+1然后结束的。只有到0的时候，才会“沉睡进程”以及“唤醒进程”。这里是主要区别。)
 */

struct condition_t{
	struct semaphore_t x_sem;	//资源等待队列(一个)
	int x_count;				//资源等待队列上的等待pcb数
	struct monitor_t *monitor;		//指向宿主monitor  方便而已。
};

struct monitor_t{
	struct semaphore_t mutex;	//入口等待队列(一个)		->	当一个进程试图进入已经占用(mutex已经锁住)的管程时候发动。这个进程会被卡在mutex的wait_queue中。
														//	互斥量，初始化为1。其余semaphore全部初始化为0。这里的mutex会在monitor::wait()中被释放掉。
	struct semaphore_t next;	//紧急等待队列(一个)		->	当某个资源已经足够，就要执行cvs[xx]->signal唤醒一个该足够资源的等待队列上的一个pcb。这时，current会被强制沉睡，放到next紧急等待队列中——优先级最高的队列。
	int next_count;				//紧急等待队列上的等待pcb数
	struct condition_t *cvs;	//资源等待队列(多个队列)	->	等着“资源”到手。这里的“资源”指的是运行权。每个cvs都包含一个信号量，这个信号量就是运行权，也就是资源。每种资源对应一个wait_queue.
														//	如果current的资源不够，那么就要执行cvs[xx]->wait来沉睡当前的current进程。然后会唤醒next上的一个急等进程。如果next没有急等pcb，那么就只能从入口mutex放一个进来了。
								//资源等待队列上的等待pcb数被封装到struct condition_t中。
};

void monitor_init(struct monitor_t *monitor, int cond_num);

void wait(struct condition_t *cond);		//因为此condition_t资源不足而调用。

void signal(struct condition_t *cond);		//因为此condition_t有资源而调用。

void enter(struct monitor_t *monitor);		//current进入管程。

void leave(struct monitor_t *monitor);		//current退出管程。

#endif /* SCHEDULE_SYNCHRONIZE_MONITOR_H_ */
