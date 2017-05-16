/*
 * VGA.h
 *
 *  Created on: 2017年2月22日
 *      Author: zhengxiaolin
 */

#ifndef VGA_VGA_H_
#define VGA_VGA_H_

#include <types.h>
#include <utils.h>
#include <x86.h>

//定义颜色
enum color
{
	black = 0,
	blue = 1,
	green = 2,
	cyan = 3,
	red = 4,
	magenta = 5,
	brown = 6,
	light_grey = 7,
	dark_grey = 8,
	light_blue = 9,
	light_green = 10,
	light_cyan = 11,
	light_red = 12,
	light_magenta = 13,
	light_brown = 14, // yellow
	white = 15
};

void move_cursor();

//得到颜色和符号的mix (字符ascii是要大于‘ ’空格字符的可输出字符。此函数内部使用。)
u16 char_mix(enum color back, enum color front, char c);

//滚动屏幕一行
void scroll();

//在屏幕显示字符。只要放到显存中就会自动显示。
void show_char(enum color back, enum color front, char c);

void clear_screen();

#endif /* VGA_VGA_H_ */
