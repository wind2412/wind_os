/*
 * entry.c
 *
 *  Created on: 2017年2月22日
 *      Author: zhengxiaolin
 */
#include <VGA.h>
#include <keyboard.h>

void init()
{
	clear_screen();
	putc('h');
	putc('e');
	putc('l');
	putc('l');
	putc('o');
	putc('\n');

	printf("hahha%d %c %x %o %s\n", 3, 'm', 0x8c, 020, "hahaha");
//	put_int(3214, 10);
//	putc('\n');

	uint8_t c = getchar();
	putc(c);
	c = getchar();
	putc(c);
	c = getchar();
	putc(c);
	c = getchar();
	putc(c);

	while(1);
}
