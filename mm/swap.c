/*
 * swap.c
 *
 *  Created on: 2017年6月3日
 *      Author: zhengxiaolin
 */

#include "../include/swap.h"

struct ide_t swap = {
	.ideno = 1,
};

void waitdisk(void) {
	// Wait for disk ready.
	while ((inb(0x1F7) & 0xC0) != 0x40)
		;
}

void swap_init()
{
	//step 0: wait ready
	waitdisk();
	//step 1: select drive
	outb(0x1F6, 0xE0 | ((swap.ideno & 1) << 4));
	waitdisk();
	//step 2: send ATA identity command
	outb(0x1F7, 0xEC);
	waitdisk();

	//read identification space of this device
	u32 buffer[128];
	insl(0x1F0, buffer, sizeof(buffer));
	swap.cmdsets = *(u32 *)((u32)buffer + 164);
	if(swap.cmdsets & 0x4000000){
		swap.sect_num = *(u32 *)((u32)buffer + 200);
	}else{
		swap.sect_num = *(u32 *)((u32)buffer + 120);
	}

	printf("swap's sect_num: %d\b", swap.sect_num);
}

// Read a single sector at offset into dst.		//offset is the sector number. --by wind.
void readsect(void *dst, u32 offset, int ide_no, int sect_count) {
	// Issue command.
	waitdisk();
	outb(0x1F2, sect_count);   // sect count
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0 | ((ide_no & 0x1) << 4) );
	outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

	// Read data.
	for(; sect_count > 0; sect_count --, dst += SECTSIZE){
		waitdisk();
		insl(0x1F0, dst, SECTSIZE / 4);
	}
}

void writesect(void *src, u32 offset, int ide_no, int sect_count) {
	// Issue command.
	waitdisk();
	outb(0x1F2, sect_count);   // sect count
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0 | ((ide_no & 0x1) << 4) );
	outb(0x1F7, 0x30);  // cmd 0x30 - write sectors

	// Read data.
	for(; sect_count > 0; sect_count --, src += SECTSIZE){
		waitdisk();
		outsl(0x1F0, src, SECTSIZE / 4);
	}
}



