/*
 * proc.c
 *
 *  Created on: 2017年6月8日
 *      Author: zhengxiaolin
 */

#include <proc.h>

struct pcb_t *idle, *init_proc;	//内核线程0，1

struct pcb_t *current = NULL;		//当前进程

struct list_node proc_list;		//进程双向链表

int proc_num;

/****************中断的处理(atomic)*************/
u32 read_eflags()
{
	u32 eflags;
	asm volatile ("pushfl; popl %0;":"=r"(eflags));
	return eflags;
}

int atom_disable_intr()
{
	if((read_eflags() & 0x200) != 0){		//此时eflags设定允许了中断。
		cli();		//关中断
		return 1;
	}
	return 0;
}
void atom_enable_intr(int flag)
{
	if(flag)
		sti();
}

/***************位图的pid处理*******************/
u32 pids[128];		//位图。

int get_pid()		//得到一个新的pid
{
	int bitmap_num = sizeof(pids)/sizeof(u32);		//实际上就是128
	for(int i = 0; i < bitmap_num; i ++){
		if(pids[i] != 0xFFFFFFFF){
			for(int j = 0; j < 32; j ++){
				if(get_bit(&pids[i], 31-j) == 0){		//如果从左->右某一位为0，即中标啦～
					set_bit(&pids[i], 31-j, 1);
					return i*32+j;
				}
			}
		}
	}
	return -1;		//全满，get_pid()失败。
}

/*******************进程相关********************/
struct pcb_t *create_pcb()
{
	struct pcb_t *pcb = (struct pcb_t *)malloc(sizeof(struct pcb_t));
	extern struct pde_t *pd;
	if(pcb == NULL)	return NULL;

	pcb->state = TASK_READY;
	pcb->pid = -1;
	pcb->start_stack = 0;
	pcb->mm = NULL;
	memset(&pcb->context, 0, sizeof(struct context));
	pcb->backup_pde = pd;
	pcb->parent = NULL;
	pcb->waitstate = NOT_WT;
	pcb->yptr = pcb->cptr = pcb->optr = NULL;
	//node没有初始化		//也不需要初始化～
	return pcb;
}

int default_proc_fn(void *what_you_want_to_say)
{
	printf("hello, proc No.1! %s.\n", (const char *)what_you_want_to_say);
	return 0;		//errorCode. 由于fn返回之后，会把return值装载在eax中，因此可以充当errorCode。
}

int fn_init_kern_thread(void *arg)		//init进程执行的函数。
{
	int pid = kernel_thread(fn_sec_kern_thread, NULL, 1);	//create proc No.2 and share mm.

//	schedule();		//这时候还没开始运行创建的新线程。因此调度一下才行。否则init进程就要退出了。===> 这段话是waitpid没写完之前写的。
	printf("init is waiting...pid %d\n", pid);
	return do_waitpid(pid);
}

int fn_sec_kern_thread(void *arg)		//进程2执行的函数
{
	int errorCode;
	extern int user_main();
	asm volatile ("int $0x80;":"=a"(errorCode):"d"(4), "b"(user_main));		//系统调用execve函数，虽然这是一个内核进程，但是还是evecve一个用户程序。和linux开机时在内核态execve一个/bin/bash是一样的。
	return errorCode;
}

int recycle_child(struct pcb_t *child)
{
	printf("waitpid %d end!\n", child->pid);
	//remove links
	if(child->yptr != NULL)		child->yptr->optr = child->optr;	else	child->parent->cptr = child->optr;	//没有更年轻的，就只能给年老的了。
	if(child->optr != NULL)		child->optr->yptr = child->yptr;
	//free stack
	free_page(la_addr_to_pg(child->start_stack - PAGE_SIZE * KTHREAD_STACK_PAGE), KTHREAD_STACK_PAGE);
	//free pcb
	list_delete(&child->node);
	free(child);
	return 0;
}

//其实就是找到zombie子进程并进行zombie子进程的stack和pcb的回收而已。
int do_waitpid(int pid)
{
	printf("waitpid %d begin...\n", pid);
	int has_child = 0;
	struct list_node *begin = proc_list.next;
	if(pid == 0){		//如果是0，就随便选择current的子进程中的一个zombie
//		while(1){
			while(begin != &proc_list){
				struct pcb_t *chd = GET_OUTER_STRUCT_PTR(begin, struct pcb_t, node);
				if(chd->parent == current){
					has_child = 1;
	inner_loop:		if(chd->state == TASK_ZOMBIE)	return recycle_child(chd);
				}
				begin = begin->next;
			}
			if(has_child == 0) 	{
				printf("waitpid wrong!\n");
				return -1;		//出错。current根本没有子进程。
			}
			else				schedule();		//如果有child，而且全不是zombie，那么就schedule好了。等到什么时候schedule回来，就会继续执行这里了。
			goto inner_loop;		//能够走到这里，必然是因为begin == &proc_list，也就是返回了头而退出了while循环。因此，只要goto那里就可以了。从头开始。
//		}
	}else{
		while(begin != &proc_list){
			struct pcb_t *chd = GET_OUTER_STRUCT_PTR(begin, struct pcb_t, node);
			if(chd->pid == pid){		//找到
				if(chd->parent == current){	//符合
					while(1){
						if(chd->state == TASK_ZOMBIE)	return recycle_child(chd);
						else							schedule();
					}
				}else{						//不符
					printf("waitpid wrong!\n");
					return -1;
				}
			}
			begin = begin->next;
		}
	}

	return 0;
}

void proc_init()
{
	list_init(&proc_list);	//初始化proc_list链表
	memset(pids, 0, sizeof(pids));

	if((idle = create_pcb()) == NULL){
		panic("can't init_proc proc 0... wrong.\n");
	}

	idle->pid = get_pid();		//这个pid一定是0.
	extern u8 kern_stack[];
	idle->start_stack = (u32)kern_stack + KTHREAD_STACK_PAGE * PAGE_SIZE;
	idle->state = TASK_READY;

	current = idle;

	proc_num ++;

//	if((kernel_thread(fn_init_kern_thread, NULL, 0)) == -1){		//创建一个进程1，init进程。init进程里边还会再创建一个进程2.作为用户进程的“躯体”
//		panic("can't init_proc proc 1... wrong.\n");
//	}
//
//	init_proc = GET_OUTER_STRUCT_PTR(proc_list.next, struct pcb_t, node);

}

//内核线程强制设置好了一些东西，eip什么的都是强制指定的。而如果用户执行的fork()，只有do_fork，且do_fork的frame不是自己设置（指定）的，而是直接复制当前用户的进程frame的。因此，是“完全复制”啊。
int kernel_thread(int (*fn)(void *), void *arg, u32 flags)
{
	struct idtframe frame;
	memset(&frame, 0, sizeof(frame));
	frame.cs = 0x08;						//设置idtframe。这个idtframe放在pcb->start_stack的最高地址处。里边有将要恢复的各种变量。之所以要利用中断，是因为后边系统调用需要中断，这里一起用了吧。
	frame.my_eax = frame.ss = 0x10;		//【全是kernel的CS和SS、DS！】
	frame.ebx = (u32)fn;
	frame.edx = (u32)arg;
	frame.eip = (u32)kernel_thread_entry;	//在switch_to之后，eip会被设置为copy_thread内部pcb->context.eip。此eip设置为中断my_push的中间后部pop恢复寄存器部分。
											//届时iret时，eip会被恢复成frame.eip==>kernel_thread_entry.然后就会执行ebx，edx中的fn函数了。之后线程消亡。
	return do_fork(flags, 0, &frame);
}

//实际上，do_fork只深拷贝了一份页表。其余的全是浅拷贝。
int do_fork(u32 flags, u32 stack_offset, struct idtframe *frame)		//这个do_fork其实并不算fork。它的pcb全是通过额外传入的参数frame设置的。如果是fork，理当从current->frame设置吧。
{
	if(proc_num > MAX_PROCESS)	return -1;

	//创建pcb
	struct pcb_t *pcb = create_pcb();
	if(pcb == NULL)		return -1;

	//填充pcb
	if(set_kthread_stack(pcb, stack_offset) == -1){		//这里，其实是设置进程的内核栈！！
		free(pcb);
		return -1;
	};
	pcb->pid = get_pid();		//这里应该关中断----> atom
	if(pcb->pid == -1){
		free(pcb);
		free_page(la_addr_to_pg(pcb->start_stack), KTHREAD_STACK_PAGE);
		return -1;
	}
	pcb->parent = current;
	list_insert_after(&proc_list, &pcb->node);		//插入node
	proc_num ++;
	copy_mm(pcb, flags, current->mm);
	copy_thread(pcb, stack_offset, frame);												//在这里，另一个进程的指针已经设置好了。就等着schedule轮到他了。
	//这里要考虑全面！！如果父进程已经fork过其他的子进程的话
	pcb->optr = pcb->parent->cptr;		//设置自己的哥哥
	pcb->parent->cptr = pcb;			//设置成为爹爹的大儿子（
	pcb->optr->yptr = pcb;				//设置哥哥的弟弟

	wakeup_process(pcb);

	//代码段不需要复制。因为直接只读～
	//由于trapframe中复制了所有的寄存器和指针(虽然没有复制代码段和数据段，但是指向了同一个位置)，因此后边的执行流程也是和parent一样的，因此被schedule之后，后边的判断也会两个都执行。所以就可以判断不同了。
	//后边的代码父子都是一样的，不同的只是全局变量current～

	//其实fork并不是“有两个返回值”，而是因为有两个进程，从而其实是有“两个fork”。C语言有且仅有一个返回值。

	printf("current: pid=> %d\n", current->pid);		//这里看看有什么问题。
	printf("pcb in do_fork: pid=> %d\n", pcb->pid);

	return pcb->pid;	//返回子进程pid

}

void do_exit(int errorCode)
{
	int flag = atom_disable_intr();		//一定要关闭中断啊！！要不这里会变得诡异了。时钟中断的插入会很有意思的......
	{
		printf("process %d finished. \n", current->pid);
		printf("=======================\n");
		delete_mm(current->mm);		//只不过此pcb块还没有被free掉。zombie进程。
		current->state = TASK_ZOMBIE;
		send_chld_to_init();
	}
	atom_enable_intr(flag);
	schedule();		//换走咯！
	panic("hahaha!!exit!!\n");		//不会执行。
}

/*参加https://wenku.baidu.com/view/8202dd7602768e9951e73811.html第22页
  各种pcb的ptr：用于描述进程的族亲关系。
	pptr：指向的是当前pcb的父进程					cptr：指向的是当前pcb的【最新fork的】子进程
	yptr：指向比当前进程年龄小(晚创建)的进程(弟弟)		optr：指向比自己年长的进程(早创建)(哥哥)
*/
void send_chld_to_init()
{
	struct pcb_t *parent = current->parent;
	if(parent->waitstate == WT_CHILD){		//如果parent调用了do_wait()函数进行等待的话，就唤醒它。
		wakeup_process(parent);
	}

	while(current->cptr != NULL){		//把所有活着的儿子全都托管给init进程.
		struct pcb_t *child = current->cptr;
		current->cptr = child->optr;						//但是当前执行的进程还是不会变的。还是这个即将死亡的进程。
		child->yptr = NULL;									//不再有其他弟弟。因为将要成为init的首徒，不需要再有弟弟。
		child->optr->yptr = NULL;							//把自己哥哥的弟弟解除关系。即在父亲的所有儿子里剔除自己。
		child->optr = init_proc->cptr;						//自己的哥哥设置为init的第一个子进程。即init子进程中最年轻的。
		if(init_proc->cptr)	init_proc->cptr->yptr = child;	//设置现在(init的第二个子进程)的哥哥的弟弟是自己
		init_proc->cptr = child;							//设置init的首徒为自己
		child->parent = init_proc;							//三姓家奴（
		if(child->state == TASK_ZOMBIE && init_proc->waitstate == WT_CHILD){
			wakeup_process(init_proc);
		}
	}
}

void delete_mm(struct mm_struct *mm)
{
	if(mm == NULL)	return;

	mm->num -= 1;	//引用计数 - 1
	extern struct pde_t *pd;
	//重新设置页目录表!!
	asm volatile ("movl %0, %%cr3"::"r"(pd));

	if(mm->num == 0){		//真·remove
		//先处理vma虚拟内存
		struct list_node *begin = mm->node.next;
		while(begin != &mm->node){
			struct vma_struct *vma = GET_OUTER_STRUCT_PTR(begin, struct vma_struct, node);
			//先free整个vma对应的所有page
			u32 vma_start = vma->vmm_start;
			while(vma_start < vma->vmm_end){
				struct pte_t *pte = get_pte(mm->pde, vma_start, 0);
				if(pte == NULL){		//如果pde没有表项的话==>跳过1024个页
					vma_start += 1024 * PAGE_SIZE;
				}else if(pte->page_addr != 0){
					unmap(mm->pde, vma_start);
					vma_start += PAGE_SIZE;
				}else{
					vma_start += PAGE_SIZE;
				}
			}
			//再free页目录表pde所malloc出来的pte
			vma_start = vma->vmm_start - vma->vmm_start % (1024 * PAGE_SIZE);	//这个比较难想。from ucore。
			while(vma_start < vma->vmm_end){
				if(mm->pde[vma_start >> 22].sign & 0x1){	//存在位......别忘了。这是细节。
					extern struct Page *pages;
					free_page(&pages[mm->pde[vma_start >> 22].pt_addr], 1);	//删除页目录表alloc对应的pte的page
					mm->pde[vma_start >> 22].pt_addr = 0;
					vma_start += PAGE_SIZE;
				}else{
					vma_start += PAGE_SIZE;
				}
			}
			begin = begin->next;
		}
		//free整个malloc出来的页目录表pde
		free_page(la_addr_to_pg((u32)mm->pde), 1);
		//free mm结构体的vma
		struct list_node *vma_begin = mm->node.next;
		while(vma_begin != &mm->node){
			struct list_node *temp = vma_begin;
			vma_begin = vma_begin->next;
			list_delete(temp);
			free(GET_OUTER_STRUCT_PTR(temp, struct vma_struct, node));
		}
		//free mm结构体
		free(mm);
	}
}

int copy_mm(struct pcb_t *pcb, int is_share, struct mm_struct *copied_mm){		//用户进程。fork的调用函数		//bug..
	struct mm_struct *old_mm = copied_mm;
	if(old_mm == NULL)	return 0;
	if(is_share){		//指针指向同样mm即可
		pcb->mm = old_mm;
		old_mm->num += 1;	//引用计数自增
		pcb->backup_pde = old_mm->pde;
		return 0;
	}else{		//必须复制一份。
		struct mm_struct *mm = (struct mm_struct *)malloc(sizeof(struct mm_struct));
		memcpy(mm, old_mm, sizeof(struct mm_struct));		//混账。。。写成了memcpy(&mm, &old_mm, sizeof(struct mm_struct));调了一个晚上。。..
		list_init(&mm->node);		//必须重新init一遍！！否则，mm->node的prev和next信息还是old_mm的！！这会造成下边没法插入！想要复制old_vmm并插入，就必须清除mm->node信息。
		pcb->mm = mm;

		//调了一个上午......发现没复制页目录表......这样，后边的复制页表的时候.....map()函数因为基于原来的进程页目录表修改，所以原先已有的页表会触发upmap()....于是原先的进程就出错了......
		//把页目录表弄过来。
		struct Page *new_pde = alloc_page(1);
		mm->pde = (struct pde_t *)pg_to_addr_pa(new_pde);		//设置新的pde......	//卧槽。。。。改成pa才可以！！！。。。。简直。。。。调的要吐血了
		memcpy(mm->pde, old_mm->pde, PAGE_SIZE);
		pcb->backup_pde = mm->pde;

		//copy vmm
		struct list_node *begin = old_mm->node.next;
		while(begin != &old_mm->node){
			struct vma_struct *old_vma = GET_OUTER_STRUCT_PTR(begin, struct vma_struct, node);

			printf("mm.....%x\n", mm);
			printf("old_vma->vmm_start:%x, old_vma->vma_end:%x\n", old_vma->vmm_start, old_vma->vmm_end);

			create_vma(mm, old_vma->vmm_start, old_vma->vmm_end, old_vma->flags);		//创建并插入vma


			//不做错误检查了。简直。。。也没有时间了。
			u32 start = old_vma->vmm_start;
			while(start < old_vma->vmm_end){
				struct pte_t *old_pte = get_pte(old_mm->pde, start, 0);
				if(old_pte == NULL){
					start += 1024 * PAGE_SIZE;
					continue;
				}
				if(old_pte->sign & 0x1)	{		//如果old_pte不是虚拟内存，则创建start对应的pte。[否则不创建？？？]
					//这里有大bug。由于pde是复制的，但是pde中的pte的指向还是old和new_pde指向同一个alloc的pte页的。所以如果unmap的话，也会发生故障啊。
					//解决方法就是，把这个pte页也复制一个。
					struct pte_t *pte = get_pte(mm->pde, start, 1);
					if(pte == old_pte){		//如果相等，就说明pte和old_pte指向同一处了。这样，在这里的fork的子进程配置中修改会影响到current进程的页表！！
						mm->pde[start >> 22].sign = 0x4|0x2;	//把pde中对应的pte表项的present位给抹除。
						pte = get_pte(mm->pde, start, 1);		//把pde抹除之后，就可以重新分配pte了。重新映射！
						//现在整个mm->pde[start>>22].pt_addr这个新alloc的页，本来是要存放1024个页表的，但是现在全是空的！！要copy过来啊......
						//算出pde中start对应那项alloc的pte页的内存地址！
						memcpy((void *)((mm->pde[start >> 22].pt_addr << 12) + VERTUAL_MEM), (const void *)((old_mm->pde[start >> 22].pt_addr << 12) + VERTUAL_MEM), PAGE_SIZE);
					}
					u32 old_page_addr_la = get_pg_addr_la(old_pte);		//被复制的page地址
					struct Page *page = alloc_page(1);
					u32 page_addr_pa = pg_to_addr_pa(page);				//新页page地址
					u32 bitsign = (old_pte->sign & 0x3) | 0x4;			//加上user态
					map(mm->pde, start, page_addr_pa, bitsign);			//map新alloc的page
					memcpy((void *)pg_to_addr_la(page), (const void *)old_page_addr_la, PAGE_SIZE);	//复制整个页。
				}
				start += PAGE_SIZE;
			}
			begin = begin->next;
		}
	}


	return 0;
}

void copy_thread(struct pcb_t *pcb, u32 offset, struct idtframe *frame)	//改成了offset。由于我糟糕的设计缘故，offset这个东西，只要不为0，那么就表示【用户态】stack(高地址栈顶)设置的时候，需要【向下】偏移的offset。因为如果是用户态，可能要execve，就要在最后4字节放上exit_proc....相反地，内核态并不需要execve来强行挪动eip指向一个code_page导致无法返回.....所以offset为0就好。
{
	//设置自己的start_stack为0x07。因为这之前可能并没有建立好mm。
	if(offset){		//offset有值，说明是用户进程了。需要改一下页表，设为0x07。		//因为一共才分配两页，因此就硬编码了。
		get_pte(pcb->mm->pde, pcb->start_stack + 4 - PAGE_SIZE, 0)->sign |= 0x4;		//设为用户。
		get_pte(pcb->mm->pde, pcb->start_stack + 4 - PAGE_SIZE * 2, 0)->sign |= 0x4;	//设为用户。
	}


	pcb->frame = (struct idtframe *)(pcb->start_stack - sizeof(struct idtframe));		//frame->esp是“利用中断后方恢复”时必要的frame指针位置。通过frame里边设置的寄存器值，进行恢复。
	//先把整个idtframe放到pcb->start_stack的最高地址处。然后设置。
	*(pcb->frame) = *frame;
	//设置frame->esp。虽然这个在内核态没啥大用。这个frame->esp会被用到“iret=>内核态切换到用户态 由iret恢复”。
//	pcb->frame->esp = stack;
	if(offset)
		pcb->frame->esp = pcb->start_stack + 4 - (ROUNDUP(offset) - offset);		//这个的意思是，根据user.c的fork()传进的参数是frame->esp而改。这个pcb->frame->esp值是0x80中断要返回的值。
														//通过计算传来的offset，也就是fork()中的frame->esp距离它自己所在的execve定下的堆栈的start_stack栈顶的偏移量，算出此被do_fork的进程这个时候应该设置frame->esp在哪里。
														//这里很牵强。。。因为如果执行的函数大小越过了一页，就肯定会错误。设计问题。唉
														//被逼无奈.....必须在offset的那个位置存放一发exit_proc。。。
														//这个frame->esp实际上是内核态-->用户态所设置的转移地址，即系统调用的返回地址。本来用户态fork的进程，在ucore中fork()函数设置此frame->esp为父进程的堆栈。然而如果用户进程中进行了系统调用，那么int $0x80结束的时候，就会执行这个frame->esp，那么堆栈会强制蹦到父进程的堆栈。这样不好吧......或者是我的理解问题。
														//所以我改成了“跳回自己的堆栈”。但是，由于回到用户态，可能会让栈页无法访问。所以根据flag，我把用户进程的栈设置成为了0x07了。
	else pcb->frame->esp = 0;		//正常情况用不到。不是必须要从内核态->用户态（0x80中断）的话，用不上。
	pcb->frame->eflags |= 0x200;		//开启中断！！否则，时钟中断竟然不好使！！

	pcb->context.esp = (u32)pcb->frame;		//注意传入的是frame的位置。但是要提取出来frame->myeax的话，需要解下引用～毕竟是指针，访问需要frame->myeax.汇编的话，如果把它放到%esp中，访问myeax需要是0(%esp).==> *(esp + 0)
	extern u32 context_to_intr;		//C语言extern一个.globl字段的话，会自动得到内部的data值。如果需要得到.globl字段的地址，还要取&.....http://blog.csdn.net/smstong/article/details/54405649
	pcb->context.eip = (u32)&context_to_intr;		//汇编函数。
}

void kernel_thread_entry()
{
	asm volatile("push %edx; call *%ebx; push %eax; call do_exit;");	//无限循环就好。因为根本没法返回，根本不知道返回哪里。然后调度的时候调走就好了。这个进程就没啦。
}

int set_kthread_stack(struct pcb_t *pcb, u32 offset)		//offset即使
{
	struct Page *pages = alloc_page(KTHREAD_STACK_PAGE);		//分配内核线程栈空间
	if(pages == NULL)	return -1;

	pcb->start_stack = pg_to_addr_la(pages) + KTHREAD_STACK_PAGE * PAGE_SIZE;	//下边还要-4...
	//忽略了......这时mm还没有建立。因此下边的判断放到copy_thread中了。
//	if(offset){		//offset有值，说明是用户进程了。需要改一下页表，设为0x07。		//因为一共才分配两页，因此就硬编码了。
//		get_pte(pcb->mm->pde, pcb->start_stack - PAGE_SIZE, 0)->sign |= 0x4;		//设为用户。
//		get_pte(pcb->mm->pde, pcb->start_stack - PAGE_SIZE * 2, 0)->sign |= 0x4;	//设为用户。
//	}
	pcb->start_stack -= 4;		//这里减4......
	if(offset){
		extern int exit_proc();
		*(u32 *)(pcb->start_stack) = (u32)exit_proc;		//放置exit_proc函数......（
	}

	return 0;
}

void wakeup_process(struct pcb_t *pcb)
{
	pcb->state = TASK_RUNNABLE;
}

void run_thread(struct pcb_t *pcb)
{
	if(current == pcb)	return;

	extern struct tss tss0;

	struct pcb_t *prev = current;
	current = pcb;
	//1.设置tss的ring0的esp0为内核线程栈的最高地址
	tss0.esp0 = pcb->start_stack;		//pcb->start_stack是do_fork所alloc的进程的【内核栈！】
	//2.设置页目录表
	asm volatile ("movl %0, %%cr3"::"r"(pcb->backup_pde));
	//3.切换进程上下文
	printf("switch to pid %d, now eip: %x, esp: %x\n", current->pid, current->context.eip, current->context.esp);
	switch_to(&prev->context, &current->context);
}

void print_thread_chains()
{
	struct list_node *begin = proc_list.prev;		//其实pcb块是倒着放的。。。所以要从prev索引.....
	printf("in pid %d:",current->pid);
	while(begin != &proc_list){
		struct pcb_t *pcb = GET_OUTER_STRUCT_PTR(begin, struct pcb_t, node);
		printf("pid %d, status %d. || ", pcb->pid, pcb->state);
		begin = begin->prev;
	}
	printf("\n");
}
