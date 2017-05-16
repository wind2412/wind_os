/*
 * keyboard.h
 *
 *  Created on: 2017年2月23日
 *      Author: zhengxiaolin
 */

#include <stdio.h>

#ifndef KEYBOARD_KEYBOARD_H_
#define KEYBOARD_KEYBOARD_H_

#define 	NIL 		0


//定义256个键位 普通
//注意：上下左右键等是双键值，一开始读取会读取到0xE0，之后需要再读取一次，才可以取到真实代表的key
static u8 keyboard_origin[0xFF] = {
	NIL,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 						//first row
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',							//second row
	NIL,/*0x1D left control*/
	'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', NIL/*0x2A left shift*/, '\\', 	//third row
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', NIL/*0x36 right shift*/, 						//fourth row
	/*其他全都不设置(全0)。特殊的双键位单独设置。*/
	NIL,  NIL, ' ',
};

//shift键开启后
static u8 keyboard_shift[0xFF] = {
	NIL,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 						//first row
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',							//second row
	NIL,/*0x1D left control*/
	'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', NIL/*0x2A left shift*/, '|', 		//third row
	'Z',  'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', NIL/*0x36 right shift*/, 					//fourth row
	NIL,  NIL, ' ',
	/*其他全都不设置(全0)。特殊的双键位单独设置。*/
};

//双键位的后一个键位：
#define		PRESS_UP			0x48
#define		PRESS_LF			0x4B
#define		PRESS_RT			0x4D
#define		PRESS_DN			0x50

#define		PRESS_CAPS_LOCK		0x3A
#define		PRESS_LF_SHIFT		0x2A
#define		PRESS_RT_SHIFT		0x36


u8 letter_to_upper_case(u8 c)	//适用于caps lock打开时，将小写转化为大写。
{
	return (c - 32);
}

void kbd_init()
{
	//留待设置键盘中断
}

u8 get_raw_msg()
{
	u8 data;
	while((data = inb(0x60)) == 0);
	putc('!');
	return data;
}

u8 getchar()
{
	u8 c = get_raw_msg();
	return c;
}

#endif /* KEYBOARD_KEYBOARD_H_ */
