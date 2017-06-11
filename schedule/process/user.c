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

int sys_execve(u32 arg[])		//假设我们的execve函数只执行一个函数。		//此函数还在内核模式中。但是，只要出去由中断恢复了，那么就会变成用户态了。
{
//	arg[0](arg[1]);	//使用execve调用用户态函数。(这里即user_main)

	//创建用户栈。
	struct Page *user_stack_pg = alloc_page(1);	//用户栈只有一页
	struct pte_t *pte = get_pte(current->backup_pde, pg_to_addr_la(user_stack_pg), 1);
	pte->page_addr = (pg_to_addr_pa(user_stack_pg) >> 12);
	pte->sign = 0x07;

	struct idtframe *frame = current->frame;	//这个frame位于stack的末尾。中断结束会被调用。
	frame->cs = 0x18|0x03;
	frame->my_eax = frame->ss = 0x20|0x03;
	frame->eflags |= 0x3000;
	frame->esp = pg_to_addr_la(user_stack_pg) + PAGE_SIZE;
	//user_main被链接在内核空间，用户禁止访问的。因此，需要把user_main给“挪动”到pg上来，然后eip跳到pg上来执行。
//	frame->eip = (u32)user_main;		//current->frame已经在之前的syscall被篡改成了中断向量的frame。因此，这里的frame实际上是中断向量0x80跳过来保存的frame。此函数设置完之后，会通过中断的后半部分pop来进行用户态的切换。

	//创建代码页，把程序复制进来
	struct Page *code_pg = alloc_page(1);
	struct pte_t *code_pte = get_pte(current->backup_pde, pg_to_addr_la(code_pg), 1);
	code_pte->page_addr = (pg_to_addr_pa(code_pg) >> 12);		//千万是pa。。。。TAT  否则会崩溃。。。
	code_pte->sign = 0x07;

	//参数就不做了......万一传进来一个神TM结构体.....莫非我还要用汇编重写吗......
	//但是为了防止execve的函数return时能够有正确的返回值，应该现在此code页的最前边push一波do_exit的eip。
	asm volatile ("movl %1, %%eax; movl %0, (%%eax);"::"r"(do_exit), "r"(pg_to_addr_la(code_pg)):"eax");		//do_exit的参数咋办......功力不过关啊...  //把do_exit的地址挪到user_main前边，为了让user_main在ret之后恢复do_exit函数到eip中，并且执行。
	memcpy((void *)(pg_to_addr_la(code_pg)+4), (void *)arg[0], PAGE_SIZE-4);		//arg[0]处指向的被执行函数，拷贝到这个页上来。   否则由于user_main在内核中，无法由用户态读取。

	frame->eip = pg_to_addr_la(code_pg)+4;	//user_main


	return 0;
}

int sys_print(u32 arg[])
{
	const char *fmt = (const char *)arg[2];
	printf("%s\n", fmt);		//调用kernel的printf函数
	return 0;
}

void print(const char *fmt)
{							//此print只支持输入字符串。			现在是在用户态。
	asm volatile ("int $0x80;"::"a"(30),"d"(fmt));		//放到edx中
}

//需要被执行的user函数
int user_main(){
	print("user_main....\n");
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


//系统调用中断，存放在user.c中。
int (*system_call[])(u32 arg[]) = {
		[1]		sys_exit,
		[2] 	sys_fork,
		[3]		sys_wait,
		[4]		sys_execve,
		[30]	sys_print,
};

//系统调用中断的handler(通用)
void system_intr(struct idtframe *frame)
{
	int syscall_num = frame->eax;
	u32 fn = frame->ebx;
	u32 argu = frame->ecx;	//假设函数有个参数。会存放在ecx中
	u32 fmt = frame->edx;
	u32 arg[5];
	memset(arg, 0, sizeof(arg));
	arg[0] = fn;
	arg[1] = argu;
	arg[2] = fmt;
	extern struct pcb_t *current;
	struct idtframe *old_frame = current->frame;		//这个篡改是专门给execve设置用的。
	current->frame = frame;		//篡改current->frame.让execve的用户态能够蹦到内核态。
	system_call[syscall_num](arg);		//呼叫内核函数
	current->frame = old_frame;
}

