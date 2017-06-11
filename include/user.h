/*
 * user.h
 *
 *  Created on: 2017年6月9日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_USER_H_
#define INCLUDE_USER_H_

#include <proc.h>

int sys_exit(u32 arg[]);
int sys_fork(u32 arg[]);
int sys_wait(u32 arg[]);
int sys_execve(u32 arg[]);
int sys_putc(u32 arg[]);

//需要被执行的user函数
int user_main();

int sys_print(u32 arg[]);

void exit_proc();

void system_intr(struct idtframe *frame);

#endif /* INCLUDE_USER_H_ */
