/*
 * productor_consumer.c
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#include <productor_consumer.h>
#include <monitor.h>


/************************ using semaphore ****************************/

int buffer_s = 0;		//设置最大为5.
struct semaphore_t empty_s;
struct semaphore_t full_s;
struct semaphore_t mutex;	//互斥量，保证对buffer这个临界区的写入是互斥的。

//为什么不能用一个变量来代表格子数：因为信号量只有“当前值”，而没有上限的设定。如果不在semaphore内部开辟一个空间表示上限的话(对semaphore自身做改动)，那么就必须使用2个semaphore，
//通过他们的和==MAX(5)，来表示上限的数目。

//生产者消费者问题。		注意：empty + full == MAX(5)
void test_semaphore()
{
	printf("begin testing semaphore... producer and consumer...\n");

	semaphore_init(&empty_s, 5);	//空槽有5个
	semaphore_init(&full_s,  0);	//满槽有0个
	semaphore_init(&mutex, 1);	//互斥量的default初始值为1.

	kernel_thread(producer_semaphore, NULL, 0);
	kernel_thread(consumer_semaphore, NULL, 0);
}

//生产者
int producer_semaphore()
{
	while(1){
//		P(&empty_s);		//减掉一个empty格子。
		if(buffer_s < 5){				//果然如此。加上这个条件判断的话，就可以少用一个信号量了。
			P(&mutex);		//得到互斥锁
			buffer_s += 1;
			printf("producer produced a product! now there has %d products~\n", buffer_s);
			V(&mutex);		//释放互斥锁
			V(&full_s);		//放掉一个full格子。
		}
	}
	return 0;
}

//消费者
int consumer_semaphore()
{
	while(1){
		P(&full_s);		//消费一个full格子
		P(&mutex);		//拿到互斥锁
		buffer_s -= 1;
		printf("consumer consumed a product! now there has %d products~\n", buffer_s);
		V(&mutex);		//释放互斥锁
//		V(&empty_s);		//放掉一个empty格子
	}
	return 0;
}


/************************** using monitor ******************************/

//其实以下这些都应该被封装到管程struct monitor中去的。

int buffer_m = 0;
struct monitor_t monitor;

#define PRODUCER 	(monitor.cvs[0])
#define CONSUMER 	(monitor.cvs[1])

void test_monitor()
{
	printf("begin testing monitor... producer and consumer...\n");

	monitor_init(&monitor, 2);			//初始化2个条件变量cv	=>	对应producer&&consumer

	kernel_thread(producer_monitor, NULL, 0);
	kernel_thread(consumer_monitor, NULL, 0);
}

int producer_monitor()
{
	while(1){
		enter(&monitor);		//current进入管程
		if(buffer_m == 5)		wait(&PRODUCER);		//buffer为满，对生产者而言，“资源不足”，沉睡生产者。(对于生产者而言，buffer不满才是资源足够的条件)
		buffer_m += 1;
		printf("producer produced a product! now there has %d products~\n", buffer_m);
		if(buffer_m == 1)		signal(&CONSUMER);		//当buffer从空变成非空之后，资源已经足够，唤醒沉睡的消费者		//卧槽！！不要忘了即使是signal里边也有P()！！可以sleep producer进程!!!
		leave(&monitor);		//current退出管程
	}
	return 0;
}

int consumer_monitor()
{
	while(1){
		enter(&monitor);		//current进入管程
		if(buffer_m == 0)		wait(&CONSUMER);		//buffer为空，资源不足，沉睡消费者
		buffer_m -= 1;
		printf("consumer consumed a product! now there has %d products~\n", buffer_m);
		if(buffer_m == 4)		signal(&PRODUCER);		//当buffer变得不再满的时候，资源已经足够，唤醒沉睡的生产者（为什么要等到是4的时候才唤醒，因为只有-1之前buffer是5，才有可能wait(&PRODUCER).这时再执行signal就没有问题。
		leave(&monitor);		//current退出管程
	}
	return 0;
}


