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
	struct idtframe *frame = current->frame;			//把当前进程的current->frame交给do_fork来复制所有寄存器！	而kern_thread中，frame是自己指定的，然后交给了do_fork进行复制了！！而用户进程还多了个mm，这就是区别！！
//	printf("frame->eip: %x\n", frame->eip);				//【深入：这里的frame->eip值其实是调用此sys_fork的int 0x80时push的eip！而这个eip值并不是人为来push的！而是CPU通过call来自动push的。而这个push的eip还被设成是trapframe的最后一部分！而且，由于是call来push，因此push的是int 0x80的“下一条”指令！！】
	////这里，在未来的do_fork中，把整个frame复制了一份。而eip停留在刚才所说的【int 0x80的下一条指令】。因此可以知道：新的进程将会从int 0x80中断完毕返回之后的下一条指令执行！！因此，这也解释了为什么这个函数总是返回子进程的同一个pid值3！！因为这个函数，并不是fork出来的新进程的执行范围！！新的进程的eip在此函数返回之后！！
	printf("frame->eax: %x\n", frame->eax);
	printf("[user fork()]\n");
	//do_fork的参数4的意思是stack栈顶要收缩4个单位存放exit_proc......而且创建的是用户进程。
//	printf("=====current->start_stack: %x\n", current->start_stack);
//	printf("=====frame->esp: %x\n", frame->esp);
//	printf("=====current->start_stack - frame->esp: %x\n", current->start_stack - frame->esp);
	int pid = do_fork(0, frame->esp/*current->start_stack - frame->esp*/, frame);		//因为do_fork的frame->esp是单独设置的。要设置得和current一样就好。		//唉，后来我可耻地改成了偏移量（
								//这里，改成了current->start_stack和frame->esp的偏移量。也就是说，届时会把偏移量平移到此fork的新子进程中。这样，两个子进程的栈才是一模一样而且完全独立的。否则如果设置为frame->esp，新的子进程的stack会跳到旧的stack中去......

	//注意：因为frame中，除了eip之外所有的值都是执行此函数之前的“过去的值”，而只有eip本身是“未来的值”。因此，此函数为了保证fork完全正确，也就是创建的新进程完全和原进程一样，那么【这个函数】就不可以改变esp什么重要寄存器的值了。这样才能保证跳出此函数的时候，current进程和进入此函数之前一样，esp都没变，但是eip已经变成当时设置的“未来的值”了。实在是太巧妙了！！
	//而，current和child进程【唯一】的不同就是，current执行了此函数，并且获得了返回值pid。而child因为直接从“eip的未来”执行，因此，并没有走此函数！！因此，eax还是原来的值！！并没有经过此函数返回！！！千万小心！！因此我的fork两次结果一次是正确的3，一次是不正确的2，正因为后边那次（子进程开启），根本没通过此函数获取返回值，而是沿用了之前保存的eax的值！！而原先eax值是2！！所以也就返回2了！！！
	//【【【解决方法】】】：所以，只要在int调用的中转函数的地方【之前】直接把eax归零就可以啦！！！无论调不调用这个函数，eax初始值都是0.如果调用此函数，eax就返回3，不调用的话，维持0就好啦～～
	//卧槽这段分析简直牛逼炸裂啊～～我都要跪服了啊哈哈哈哈！！
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
	const char *fmt = (const char *)arg[0];
	printf("%s\n", fmt);		//调用kernel的printf函数
	return 0;
}

int sys_getpid(u32 arg[])
{
	return current->pid;
}



int exit_proc()
{
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"d"(1), "b"(0));	//errorCode为0，保存在ebx中了。
	return ret;
}

int getpid();

__attribute__((always_inline)) inline int fork()		//用户态下嵌套中断不知道可不可以？
{
	int pid = 0;		//我开始就赋值返回值为0，正常一定这里会执行。只不过，如果是current父进程，接下来会走下边的int 0x80调用。但是如果是子进程，因为int触发中断，frame的eip自动被压入下一条指令的eip值，即【汇编中】0x80的下一条～总之int 0x80不会执行第二次，而是跳过。但是返回值还会赋值到pid中。。。
	asm volatile ("int $0x80;":"=a"(pid):"d"(2));
//	if(pid == current->pid)		return 0;		//current由于加载不到页上，因此得不到。但是果然还是通过0x80系统调用更好吧。

//	if(pid == 2)	print("2;;;;;;\n");		//留个回忆～～要不也不能有上面一大段分析！～
//	else if(pid == 3)	print("3;;;;;;\n");

//	if(pid == getpid())			return 0;
//	else 						return pid;
	return pid;					//直接返回就好啦～～因为已经eax清零了～原先的垃圾值不见啦～		//如果是fork产生的child进程，那么就会直接从这里执行～～
}

int waitpid(int pid)
{
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"d"(3), "c"(pid));
	return ret;
}

int getpid()
{
	int pid;
	asm volatile ("int $0x80;":"=a"(pid):"d"(5));
	return pid;
}

int print(const char *fmt)		//print()用户态，触发中断int $0x80变为内核态-->system_intr()内核态，使用-->sys_print().先是内核态. 然后从中断中返回，会恢复中断之前的现场，即回归用户态。
{							//此print只支持输入字符串。			现在是在用户态。
	int ret;
	asm volatile ("int $0x80;":"=a"(ret):"d"(30),"b"(fmt));		//放到edx中
	return ret;
}

//需要被执行的user函数
int user_main(){		//应该是2号进程				//现在的函数都是【运行时地址才能确定】的。而仅仅加载这一函数到某一页，由于会默认计算偏移量，而fork函数在此页的前几个页。因此计算的结果会造成不会跳到“真正的fork函数”，而是跳到“复制到页的fork函数”。但是我们只复制了一个页，fork并没有复制到页上，因此必然出错。
													//这里的原因在于：这里并不是elf，因此必然是“只有部分”的程序了。因此只能通过中断0x80的特殊方式，直接强行使用内核调用正确的位置，免去没有加载到页上的烦恼了。
													//所以在这个并非elf而且仅仅是“一个函数”的程序，内部调用还是都inline函数吧，直接能够加载进去。否则会各种出错的啊。
													//还有那个fork。一开始只返回pid的时候，能被-O2优化检测到函数太小，直接inline进user_main了。（实在是幸运，要不一开始就运行不了啦）但是在fork加上了条件分支之后，不再能变得default inline了。因此必须产生call函数调用，由于【页上的fork】不在页上user_main的相对偏移【指真·user_main和真·fork的相对偏移】(fork并没有加载到页上)，就必然出错啦。
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
		[5]		sys_getpid,			//此函数实际上是为了解决：因为我的用户进程不是elf文件，而只是一个函数。这样，fork()需要引用内核的current，但是由于我只引入了user_main所在的那一页，current捕捉不到。因此产生了再写一个中断的想法LOL;
		[30]	sys_print,
};

//系统调用中断的handler(通用)
void system_intr(struct idtframe *frame)
{
	int syscall_num = frame->edx;
	u32 fn = frame->ebx;
	u32 argu = frame->ecx;	//假设函数有个参数。会存放在ecx中
	u32 arg[5];
	memset(arg, 0, sizeof(arg));
	arg[0] = fn;
	arg[1] = argu;
	extern struct pcb_t *current;
	struct idtframe *old_frame = current->frame;		//这个篡改是专门给execve设置用的。
	current->frame = frame;		//篡改current->frame.让execve的用户态能够蹦到内核态。
//	if(syscall_num == 2)	asm volatile ("movl $0, %eax");			//如果是fork调用，因为上边的分析，需要让eax清空。否则现在的eax值会延续并影响到子进程开始的eax值，条件判断的时候就会误判子进程fork()返回的值为原先的垃圾值！
	//很残念以上不行。因为如果是child进程，直接从int之后一条开始执行。所以，这里也是执行不到的。必须在int之前正正好好把它清零才行啊。所以我不得不改变系统调用的默认值啊。。原先由eax获得系统中断号，现在要改改了......然而后来发现也不用～因为有fork()的局部变量pid存在啊～见fork()函数实现～
	frame->eax = system_call[syscall_num](arg);		//呼叫内核函数
	current->frame = old_frame;
}

