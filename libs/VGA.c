/*
 * VGA.c
 *
 *  Created on: 2017年2月26日
 *      Author: zhengxiaolin
 */

#include <VGA.h>

//显存初始位置
static uint16_t *VGA_START = (uint16_t *) 0xB8000;	//static变量和全局变量的【定义】要放在.c文件中。。。否则即使有宏，链接时有多个文件引用VGA.h，也会重定义多次。
													//宏只是防止【本文件】中重复引用同一文件而设置的。

//光标的当前横纵坐标(默认terminal长宽：80*25)
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;


void move_cursor()
{
	uint16_t cursor = cursor_y * 80 + cursor_x;

	//两个内部端口号用来执行汇编in指令。索引到寄存器：通过0x3D4端口；设置寄存器的值：通过0x3D5端口。
	outb(0x3D4, 14);					//通过0x3D4索引到第14个寄存器，以发送高八位。
	outb(0x3D5, cursor >> 8);	//发送高八位，需要先把高8位移动下来。
	outb(0x3D4, 15);					//索引到第15个寄存器以发送低八位。
	outb(0x3D5, cursor);
}

//得到颜色和符号的mix (字符ascii是要大于‘ ’空格字符的可输出字符。此函数内部使用。)
uint16_t char_mix(enum color back, enum color front, char c)
{
	uint8_t back_color = (uint8_t) back;
	uint8_t front_color = (uint8_t) front;
	uint16_t mix = ((back_color << 4) | (front_color & 0xf));
	return ((mix << 8) | c);
}

//滚动屏幕一行
void scroll()
{
	if (cursor_y < 25)
		return;

	for (uint8_t i = 1; i < 25; i++) {
		for (uint8_t j = 0; j < 80; j++) {
			VGA_START[(i - 1) * 80 + j] = VGA_START[i * 80 + j];
		}
	}
	for (uint8_t i = 0; i < 80; i++) {
		VGA_START[24 * 80 + i] = char_mix(black, white, ' ');
	}
}

//在屏幕显示字符。只要放到显存中就会自动显示。
void show_char(enum color back, enum color front, char c)
{
	if (c == '\b' && cursor_x != 0) {
		cursor_x--;
		move_cursor();
	} else if (c == '\t') {
		cursor_x = ((cursor_x + 4) | 0xFC);
		move_cursor();
	} else if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
		scroll();
		move_cursor();
	} else if (c >= ' ') {
		VGA_START[cursor_y * 80 + cursor_x] = char_mix(back, front, c);
		cursor_x++;
		if (cursor_x >= 80) {
			cursor_x = 0;
			cursor_y++;
			scroll();
		}
		move_cursor();
	}
}

void clear_screen()
{
	uint16_t c = char_mix(black, white, ' ');
	for (int i = 0; i < 25; i++) {
		for (int j = 0; j < 80; j++) {
			VGA_START[i * 80 + j] = c;
		}
	}
	cursor_x = cursor_y = 0;
	move_cursor();
}
