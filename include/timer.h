/*
 * timer.h
 *
 *  Created on: 2017年5月17日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

#include <types.h>
#include <x86.h>
#include <idt.h>
#include <stdio.h>


void timer_intr_init();

void timer_handler(struct idtframe *);


#endif /* INCLUDE_TIMER_H_ */
