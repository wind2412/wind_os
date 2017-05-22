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

//缓冲区，没写shell之前暂时不涉及。
u8 kbd_buff[1024];

u8 double_key_on	   = 0;		//双键位第一个0xE0是否被触发？

u8 is_lf_shift_pressed = 0;
u8 is_rt_shift_pressed = 0;
u8 is_lf_ctrl_pressed  = 0;
u8 is_rt_ctrl_pressed  = 0;
u8 is_capslock_pressed = 0;

u8 getchar()
{
	u8 c = get_raw_msg();
	switch(c){
		case PRESS_CAPS_LOCK:
			is_capslock_pressed = !is_capslock_pressed;
			break;
		case PRESS_LF_SHIFT:
			is_lf_shift_pressed = 1;
			break;
		case PRESS_RT_SHIFT:
			is_rt_shift_pressed = 1;
			break;
		case PRESS_CTRL:
			if(!double_key_on)	is_lf_ctrl_pressed  = 1;
			else				is_rt_ctrl_pressed  = 1;
			break;
		case RELEASE_LF_SHIFT:
			is_lf_shift_pressed = 0;
			break;
		case RELEASE_RT_SHIFT:
			is_rt_shift_pressed = 0;
			break;
		case DOUBLE_KEY:		//0xE0双键位。比如ctrl的释放就会触发双键位。
			double_key_on = 1;
			break;
		case RELEASE_CTRL:
			if(!double_key_on)	is_lf_ctrl_pressed = 0;
			else				is_rt_ctrl_pressed = 0;
			double_key_on = 0;	//取消设置0xE0，因为已经使用过了。
			break;
		default:
			break;
	}
	if(is_capslock_pressed){
		if(is_lf_shift_pressed || is_rt_shift_pressed)	return keyboard_origin[c-1];		//即使按的是shift也没有关系。因为keyboard_shift内部存放的对应shift位也是nil。
		else return keyboard_shift[c-1];
	}else{
		if(is_lf_shift_pressed || is_rt_shift_pressed)	return keyboard_shift[c-1];
		else return keyboard_origin[c-1];
	}
}
