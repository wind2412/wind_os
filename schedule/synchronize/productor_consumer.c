/*
 * productor_consumer.c
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#include <productor_consumer.h>

int buffer = 0;		//设置最大为5.
struct semaphore_t empty;
struct semaphore_t full;
struct semaphore_t mutex;	//互斥量，保证对buffer这个临界区的写入是互斥的。

//为什么不能用一个变量来代表格子数：因为信号量只有“当前值”，而没有上限的设定。如果不在semaphore内部开辟一个空间表示上限的话(对semaphore自身做改动)，那么就必须使用2个semaphore，
//通过他们的和==MAX(5)，来表示上限的数目。

//生产者消费者问题。		注意：empty + full == MAX(5)
void test_semaphore()
{
	printf("begin testing semaphore... producer and consumer...\n");

	semaphore_init(&empty, 5);	//空槽有5个
	semaphore_init(&full,  0);	//满槽有0个
	semaphore_init(&mutex, 1);	//互斥量的default初始值为1.

	kernel_thread(producer, NULL, 0);
	kernel_thread(consumer, NULL, 0);
}

//生产者
int producer()
{
	while(1){
		P(&empty);		//减掉一个empty格子。
		P(&mutex);		//得到互斥锁
		buffer += 1;
		printf("producer produced a product! now there has %d products~\n", full.value);
		V(&mutex);		//释放互斥锁
		V(&full);		//放掉一个full格子。
	}
	return 0;
}

//消费者
int consumer()
{
	while(1){
		P(&full);		//消费一个full格子
		P(&mutex);		//拿到互斥锁
		buffer -= 1;
		printf("consumer consumed a product! now there has %d products~\n", full.value);
		V(&mutex);		//释放互斥锁
		V(&empty);		//放掉一个empty格子
	}
	return 0;
}
