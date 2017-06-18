/*
 * semaphore.h
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#ifndef SCHEDULE_SYNCHRONIZE_SEMAPHORE_H_
#define SCHEDULE_SYNCHRONIZE_SEMAPHORE_H_

#include <proc.h>

struct semaphore_t{
	int value;
	struct list_node queue;
};

void semaphore_init(struct semaphore_t *s, int value);

void P(struct semaphore_t *s);		//wait()

void V(struct semaphore_t *s);		//signal();


#endif /* SCHEDULE_SYNCHRONIZE_SEMAPHORE_H_ */
