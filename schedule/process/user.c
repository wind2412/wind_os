/*
 * user.c
 *
 *  Created on: 2017年6月9日
 *      Author: zhengxiaolin
 */

#include <user.h>

//系统调用需要的函数。调用的本体handler在tss.h中。
int sys_exit(u32 arg[])
{

}

int sys_fork(u32 arg[])
{

}

int sys_wait(u32 arg[])
{

}

int sys_execve(u32 arg[])		//假设我们的execve函数只执行一个函数。
{
//	arg[0](arg[1]);	//使用execve调用用户态函数。(这里即user_main)
	//创建用户栈。
	struct Page *pg = alloc_page(1);	//用户栈只有一页
	struct pte_t *pte = get_pte(current->backup_pde, pg_to_addr_la(pg), 1);
	pte->page_addr = (pg_to_addr_pa(pg) >> 12);
	pte->sign = 0x07;

	struct idtframe *frame = current->frame;	//这个frame位于stack的末尾。中断结束会被调用。
	frame->cs = 0x18|0x03;
	frame->my_eax = frame->ss = 0x20|0x03;
	frame->eflags |= 0x3000;
	frame->esp = pg_to_addr_la(pg);
	frame->eip = (u32)user_main;

	return 0;
}

int sys_putc(u32 arg[])
{

}

//需要被执行的user函数
int user_main(){
	printf("user_main....\n");
	return 0;
//	int pid;
//	if((pid = fork()) != 0){
//		printf("this is the father process.\n");
//	}else{
//		printf("this is the child process.\n");
//	}
//
//	waitpid(pid);
}
