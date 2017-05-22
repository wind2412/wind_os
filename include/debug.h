/*
 * debug.h
 *
 *  Created on: 2017年5月16日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_DEBUG_H_
#define INCLUDE_DEBUG_H_

#include <types.h>
#include <elf.h>
#include <string.h>
#include <stdio.h>


//some other messages
u32 symtab_offset, strtab_offset, symtab_size/*总大小，和symtab_num不同*/, strtab_size;

void small_fgets(char *dest, long offset, long size, char *src);

void init_elf_tables();

const char *get_func_name(u32 addr);

void print_backtrace();


#endif
