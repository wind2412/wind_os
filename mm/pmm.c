/*
 * pmm.c
 *
 *  Created on: 2017年5月23日
 *      Author: zhengxiaolin
 */

#include <pmm.h>

struct e820map *mm = (struct e820map *) ( 0x8000 + VERTUAL_MEM );

void print_memory()
{
	for(int i = 0; i < mm->nr_map; i ++){
		printf("base_address: %x  length: %x  type: %d\n", mm->map[i].base_lo, mm->map[i].length_lo, mm->map[i].type);
	}
}


//copy from hx_kernel
void page_fault(struct idtframe *frame)
{
    u32 cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2));

    printf("Page fault at %x, virtual faulting address %x\n", frame->eip, cr2);
    printf("Error code: %x\n", frame->errorCode);

    // bit 0 为 0 指页面不存在内存里		//此处查阅资料，我在这里添加了U/S保护的确认。
    if ( !(frame->errorCode & 0x1)) {
        printf("Because the page wasn't present.\n");
    } else {		//如果errorCode & 0x1 == 1 则是：由于保护特权级别太高，造成无法读取，进而把bit1归0显示读错误。
    				//stackover flow: https://stackoverflow.com/questions/9759439/page-fault-shortage-of-page-or-access-violation
    	printf("the page's privilege is so high, you read it in user mode.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (frame->errorCode & 0x2) {
        printf("Write error.\n");
    } else {
        printf("Read error. The privilege of this page is so high～\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (frame->errorCode & 0x4) {
        printf("In user mode.\n");
    } else {
        printf("In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (frame->errorCode & 0x8) {
        printf("Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (frame->errorCode & 0x10) {
        printf("The fault occurred during an instruction fetch.\n");
    }

    printf("esp: %x\n", frame->esp);

    while (1);
}

void init_pmm()
{
	set_handler(14, page_fault);
	print_memory();
}
