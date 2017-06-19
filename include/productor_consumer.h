/*
 * productor_consumer.h
 *
 *  Created on: 2017年6月18日
 *      Author: zhengxiaolin
 */

#ifndef SCHEDULE_SYNCHRONIZE_PRODUCTOR_CONSUMER_H_
#define SCHEDULE_SYNCHRONIZE_PRODUCTOR_CONSUMER_H_

#include <semaphore.h>

/********** using semaphore ************/

int producer_semaphore();

int consumer_semaphore();

void test_semaphore();

/********** using monitor **************/

int producer_monitor();

int consumer_monitor();

void test_monitor();


#endif /* SCHEDULE_SYNCHRONIZE_PRODUCTOR_CONSUMER_H_ */
