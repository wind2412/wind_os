/*
 * keyboard.c
 *
 *  Created on: 2017年5月17日
 *      Author: zhengxiaolin
 */

#include <keyboard.h>

u8 letter_to_upper_case(u8 c)	//适用于caps lock打开时，将小写转化为大写。
{
	return (c - 32);
}

void kbd_handler(struct idtframe* frame)
{
	printf("interrupt %d has been called and I read %c\n", frame->intr_No, /*inb(0x60)*/getchar());
}

void kbd_init()
{
	set_handler(33, kbd_handler);		//设置了键盘的中断
	//留待设置键盘中断
}

u8 get_raw_msg()
{
	u8 data;
	while((data = inb(0x60)) == 0);
	return data;
}

u8 getchar()
{
	u8 c = get_raw_msg();
	return keyboard_origin[c-1];
}
