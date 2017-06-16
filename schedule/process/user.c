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
	sti();		//这里。由于好像不允许嵌套中断啊。所以，由于这里已经是int 0x80中断的范围了，因此进到这里，eflags寄存器的中断位(eflags|0x200)就被取消掉了。因此，要在这里手动开一下中断。要不我do_exit是一堆循环，那么时钟中断进不来啊。
	do_exit(arg[1]);
	return arg[1];		//errorCode.	但是其实是不会返回的。
}

int sys_fork(u32 arg[])
{
	struct idtframe *frame = current->frame;
	printf("[user fork()]\n");
	//do_fork的参数4的意思是stack栈顶要收缩4个单位存放exit_proc......而且创建的是用户进程。
	printf("=====current->start_stack: %x\n", current->start_stack);
	printf("=====frame->esp: %x\n", frame->esp);
	printf("=====current->start_stack - frame->esp: %x\n", current->start_stack - frame->esp);
	int pid = do_fork(0, /*frame->esp*/current->start_stack - frame->esp, frame);		//因为do_fork的frame->esp是单独设置的。要设置得和current一样就好。		//唉，后来我可耻地改成了偏移量（
								//这里，改成了current->start_stack和frame->esp的偏移量。也就是说，届时会把偏移量平移到此fork的新子进程中。这样，两个子进程的栈才是一模一样而且完全独立的。否则如果设置为frame->esp，新的子进程的stack会跳到旧的stack中去......
	printf("[fork() over]\n");
	return pid;
}

int sys_wait(u32 arg[])
{
	return do_waitpid(arg[1]);
}


//execve：当且仅当执行了execve，才会有mm！否则kern_thread的所有pcb->mm全是NULL！！！
int sys_execve(u32 arg[])		//假设我们的execve函数只执行一个函数。		//此函数还在内核模式中。但是，只要出去由中断恢复了，那么就会变成用户态了。
{
//	arg[0](arg[1]);	//使用execve调用用户态函数。(这里即user_main)

	//创建用户栈。	//就用pid=2的栈好了。————不太好，虽然execve是占有了整个stack，但是毕竟返回信息什么的还在原先pid=2的栈中。如果占用了，不一定什么后果呢。
	struct Page *user_stack_pg = alloc_page(KTHREAD_STACK_PAGE);	//用户栈也有2页吧
	struct pte_t *pte = get_pte(current->backup_pde, pg_to_addr_la(user_stack_pg), 1);
	struct pte_t *pte_2 = get_pte(current->backup_pde, pg_to_addr_la(user_stack_pg+1), 1);
	printf("====stack_pg of execve: %x\n", pg_to_addr_la(user_stack_pg));
	pte->page_addr = (pg_to_addr_pa(user_stack_pg) >> 12);
	pte_2->page_addr = (pg_to_addr_pa(user_stack_pg+1) >> 12);
	pte->sign = 0x07;
	pte_2->sign = 0x07;
//	struct Page *user_stack_pg = la_addr_to_pg(current->start_stack - KTHREAD_STACK_PAGE * PAGE_SIZE);

	struct idtframe *frame = current->frame;	//这个frame位于stack的末尾。中断结束会被调用。
	frame->cs = 0x18|0x03;
	frame->my_eax = frame->ss = 0x20|0x03;
	frame->eflags |= 0x3000;		//IO给用户开放
	frame->eflags |= 0x200;			//中断给用户开放
	frame->esp = pg_to_addr_la(user_stack_pg) + PAGE_SIZE * KTHREAD_STACK_PAGE - 4;	//最前边放着一个exit_proc函数的调用！
	//user_main被链接在内核空间，用户禁止访问的。因此，需要把user_main给“挪动”到pg上来，然后eip跳到pg上来执行。
//	frame->eip = (u32)user_main;		//current->frame已经在之前的syscall被篡改成了中断向量的frame。因此，这里的frame实际上是中断向量0x80跳过来保存的frame。此函数设置完之后，会通过中断的后半部分pop来进行用户态的切换。
//	frame->esp = current->start_stack;		//必须设置！！！因为将要从内核态切回用户态！！！所以这个frame->esp实际上是会通过iret指令来切换的！如果不设置，就错了！！

	//由于sys_exec是从内核空间出来的，因而必然pcb->mm为NULL。因此要设置个页目录表。变成用户的页目录表。
//	current->mm = create_mm(NULL);
//	struct Page *user_pde_pg = alloc_page(1);
//	struct pte_t *pde_pte = get_pte(current->backup_pde, pg_to_addr_la(user_pde_pg), 1);
//	pde_pte->page_addr = (pg_to_addr_pa(user_pde_pg) >> 12);		//必须注册！！
//	pde_pte->sign = 0x07;
//	memcpy((void *)pg_to_addr_la(user_pde_pg), current->backup_pde, PAGE_SIZE);	//复制内核的页表到用户的页表
//	current->mm->pde = (struct pde_t *)pg_to_addr_la(user_pde_pg);
//	current->backup_pde = current->mm->pde;
	extern struct mm_struct *mm;
	copy_mm(current, 0, mm);		//用正确的函数复制一下pde表。用自己写的，怕是在pte页上会出现unmap这样同样的毛病吧。

	//创建代码页，把程序复制进来
	struct Page *code_pg = alloc_page(1);
	struct pte_t *code_pte = get_pte(current->backup_pde, pg_to_addr_la(code_pg), 1);
	create_vma(current->mm, pg_to_addr_la(code_pg), pg_to_addr_la(code_pg) + PAGE_SIZE, 0x07);		//【创建一个vma，让此页可以换入换出！！】
	code_pte->page_addr = (pg_to_addr_pa(code_pg) >> 12);		//千万是pa。。。。TAT  否则会崩溃。。。
	code_pte->sign = 0x07;

	//参数就不做了......万一传进来一个神TM结构体.....莫非我还要用汇编重写吗......
	//但是为了防止execve的函数return时能够有正确的返回值，应该现在[用户栈]的最前边push一波do_exit的eip。//但是其实这样有个弊端，就是其实如果是在用户态下，cs:eip是无法访问栈的。因为虽然叫做“用户栈”，其实那个栈也只有内核态下的cs:eip和esp能访问。do_fork并没有带有那种更改用户态的接口。
	asm volatile ("movl %1, %%eax; movl %0, (%%eax);"::"r"(/*do_exit*/exit_proc), "r"(/*current->start_stack - 4*/pg_to_addr_la(user_stack_pg) + PAGE_SIZE * KTHREAD_STACK_PAGE - 4):"eax");		//do_exit的参数咋办......功力不过关啊...  //把do_exit的地址挪到user_main前边，为了让user_main在ret之后恢复do_exit函数到eip中，并且执行。
	memcpy((void *)(pg_to_addr_la(code_pg)), (void *)arg[0], PAGE_SIZE);		//arg[0]处指向的被执行函数，拷贝到这个页上来。   否则由于user_main在内核中，无法由用户态读取。
	printf("====copy code to %x\n", pg_to_addr_la(code_pg));

	frame->eip = pg_to_addr_la(code_pg);	//user_main

		//注意：原理：把user_main函数放到用户进程的[代码页]中去执行。但是把exit_proc的地址放到用户进程的[用户栈]的栈顶去执行。注意最后eip指向[代码页]中的ret，这时esp会从[用户栈]去pop！！
		//这样就能够让eip指向了exit_proc了！！注意，[代码页(代码段)]和[用户栈(数据段)]一定要分开！！

									//像是sys_execve函数，和其他不太一样。因为后来篡改了中断返回的函数，因此，本来就与其他中断不同的此函数从内核中调用（别的函数全从用户态调用），并且返回用户态。
									//别的中断0x80函数都是：从用户态调用，并且突然跳进内核态执行，然后恢复现场返回了用户态。

	//之后最后用户态的代码user_main执行完毕之后，就会exit。但是这还是在用户态。知道schedule()生效，会保存这里的东西到pcb，然后切换到另一个进程，这样的话，就会跳到另一个进程的“态”中去。因此，和是否停留在用户态没有必然的关系。

	return 0;
}

int sys_print(u32 arg[])
{
	const char *fmt = (const char *)arg[2];
	printf("%s\n", fmt);		//调用kernel的printf函数
	return 0;
}

int exit_proc()
{
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"a"(1), "b"(0));	//errorCode为0，保存在ebx中了。
	return ret;
}

int fork()
{
	int pid;
	asm volatile ("int $0x80;":"=a"(pid):"a"(2));
	return pid;
}

int waitpid(int pid)
{
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"a"(3), "c"(pid));
	return ret;
}

int print(const char *fmt)		//print()用户态，触发中断int $0x80变为内核态-->system_intr()内核态，使用-->sys_print().先是内核态. 然后从中断中返回，会恢复中断之前的现场，即回归用户态。
{							//此print只支持输入字符串。			现在是在用户态。
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"a"(30),"d"(fmt));		//放到edx中
	return ret;
}

//需要被执行的user函数
int user_main(){		//应该是2号进程
	print("user_main....\n");

	int pid;
	if((pid = fork()) != 0){		//产生3号进程
		print("this is the father process.\n");
		waitpid(3);
	}else{
		print("this is the child process.\n");
	}


	return 0;
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
	frame->eax = system_call[syscall_num](arg);		//呼叫内核函数
	current->frame = old_frame;
}

