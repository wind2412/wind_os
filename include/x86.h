/*
 * common.h
 *
 *  Created on: 2017年2月22日
 *      Author: zhengxiaolin
 */

#ifndef LIBS_X86_H_
#define LIBS_X86_H_


#include <types.h>

inline void outb(uint16_t port, uint8_t value) __attribute__((always_inline));

inline uint8_t inb(uint16_t port) __attribute__((always_inline));

inline void insl(uint32_t port, void *addr, int cnt) __attribute__((always_inline));

inline void outw(uint16_t port, uint16_t data) __attribute__((always_inline));

/*************************************************************************/

static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

inline void outb(uint16_t port, uint8_t value)
{
	asm volatile("outb %1, %0"::"d"(port), "a"(value));
}

//...c的inline 竟然必须定义在同一个头文件中。。。否则error: inlining failed in call to always_inline 'inb': function body not available
//从port端口中读取端口值，并写到eax中。
inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile("inb %1, %0":"=a"(ret):"d"(port));
	return ret;
}

inline void
insl(uint32_t port, void *addr, int cnt)
{
    asm volatile (
            "cld;"
            "repne; insl;"
            : "=D" (addr), "=c" (cnt)
            : "d" (port), "0" (addr), "1" (cnt)
            : "memory", "cc");
}

inline void
outw(uint16_t port, uint16_t data)
{
    asm volatile ("outw %0, %1" :: "a" (data), "d" (port));
}


#endif /* LIBS_X86_H_ */
