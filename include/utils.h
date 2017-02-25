/*
 * utils.h
 *
 *  Created on: 2017年2月23日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

typedef __builtin_va_list va_list;

//宏就相当于引用。因为是直接替换，形参就是真正的那个实参。
#define va_start(ap, last) 	(__builtin_va_start(ap, last))	//ap跳过了printf中的字符串，指向下一个参数地址
#define va_arg(ap, type) 	(__builtin_va_arg(ap, type))	//返回ap指向的类型，并把ap挪到下一个参数地址。
#define va_end(ap)											//把ap赋值为NULL。



#endif /* INCLUDE_UTILS_H_ */
