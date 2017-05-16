/*
 * stdio.c
 *
 *  Created on: 2017年2月26日
 *      Author: zhengxiaolin
 */

#include <VGA.h>

/*********** printf ************/

void putc(char c)
{
	show_char(black, white, c);
}

void put_raw_string(const char *c)
{
//	while(c){	//mdzz!....
	while(*c){
		show_char(black, white, *c);
		c++;
	}
}

void put_int(u32 num, u32 base)
{
	static const char *table = "0123456789abcdef";

	char buf[11];	//32位整数最高10位
	int cnt = 0;
	int bit;		//(个，十...)位的值

//	while((bit = num % base) != 0){		//雾。。一定要是数字本身是0.不是求得的位数是0.。。bug...
//		buf[cnt++] = table[bit];
//		num /= base;
//	}

	while(num != 0){
		bit = num % base;
		buf[cnt++] = table[bit];
		num /= base;
	}

	for(int i = 0; i <= (cnt-1)/2; i ++){
		char tmp = buf[i];
		buf[i] = buf[cnt-1-i];
		buf[cnt-1-i] = tmp;
	}

	buf[cnt] = '\0';

	if(base == 16){
		put_raw_string("0x");
	}
	if(base == 8){
		put_raw_string("0");
	}

	put_raw_string(buf);
}


void printf(const char *fmt, ...)
{
	const char *ptr = fmt;
	va_list ap;
	va_start(ap, fmt);	//ap指向“...”的第一个参数
	while(*ptr != '\0'){
		if(*ptr != '%'){
			putc(*ptr++);
			continue;
		}
		//bug fixed:
		ptr ++;	//在这里加上就ok。
		if(*/*++*/ptr == 'c'){		//低级错误。。如果在每个if elseif 判断时都写++ptr，到判断最后一个。。ptr会+5.因此一定不要在每个if elseif当中写++！！
		//	char arg = va_arg(ap, char);	//错误！！！ va_arg的陷阱：因为调用者不会向边长参数(比如printf)传入char，short等参数，其实他们最后全变成了int。
											//并不理解这种设计思想。但是请参见：http://www.cppblog.com/ownwaterloo/archive/2009/04/21/unacceptable_type_in_va_arg.html
			int arg = va_arg(ap, int);//应该用这个！！！
			putc(arg);
		}else if(*/*++*/ptr == 's'){
			const char *arg = va_arg(ap, char *);
			put_raw_string(arg);
		}else if(*/*++*/ptr == 'd'){
			u32 arg = va_arg(ap, u32);
			put_int(arg, 10);
		}else if(*/*++*/ptr == 'x'){
			u32 arg = va_arg(ap, u32);
			put_int(arg, 16);
		}else if(*/*++*/ptr == 'o'){
			u32 arg = va_arg(ap, u32);
			put_int(arg, 8);
		}
		ptr++;
	}
	va_end(ap);		//对于va_end的调用说明：详见http://www.cppblog.com/ownwaterloo/archive/2009/04/21/is_va_end_necessary.html
}
