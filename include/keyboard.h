/*
 * keyboard.h
 *
 *  Created on: 2017年2月23日
 *      Author: zhengxiaolin
 */


#ifndef KEYBOARD_KEYBOARD_H_
#define KEYBOARD_KEYBOARD_H_

#include <types.h>
#include <idt.h>
#include <x86.h>
#include <stdio.h>

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
#define		PRESS_UP			0x48		//双中断。
#define		PRESS_LF			0x4B		//双中断。
#define		PRESS_RT			0x4D		//双中断。
#define		PRESS_DN			0x50		//双中断。

#define		DOUBLE_KEY			0xE0		//这个是双键位

#define		PRESS_CAPS_LOCK		0x3A

#define		PRESS_CTRL			0x1D		//这是双中断，先中断0xE0. 左右control按下是一样的。只不过右control是双中断而已。
#define 	RELEASE_CTRL		0x9D
//#define 	RELEASE_RT_CTRL		0x9D		//这也是双中断，先中断0xE0

#define		PRESS_LF_SHIFT		0x2A
#define		PRESS_RT_SHIFT		0x36
#define		RELEASE_LF_SHIFT	0xAA
#define		RELEASE_RT_SHIFT	0xB6


//适用于caps lock打开时，将小写转化为大写。
u8 letter_to_upper_case(u8 c);

void kbd_handler(struct idtframe* frame);

void kbd_init();

u8 get_raw_msg();

u8 getchar();

#endif /* KEYBOARD_KEYBOARD_H_ */
