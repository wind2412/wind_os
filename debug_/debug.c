/*
 * debug.c
 *
 *  Created on: 2017年5月21日
 *      Author: zhengxiaolin
 */

#include <debug.h>
#include <pmm.h>


//set up elf_hdr and get how many sec_hdr there are
struct elf_t *elf_hdr;

struct sec_t *sec_ptr;
int sec_num;

struct prog_t *prog_ptr;
int prog_num;

struct symtable_t *symtable_ptr;
int symtable_num;

const char *strtab_ptr;


//向dest填充size个字节，从src那里。
void small_fgets(char *dest, long offset, long size, char *src)
{
    char *begin = src + offset;
    while(size > 0){
        *dest++ = *begin++;
        size --;
    }
}

void init_elf_tables()
{
	elf_hdr = (struct elf_t*) ( 0x10000 + VERTUAL_MEM );
	//bootmain只读了4096个字节。
//	int page_size = 4096;      //如果程序崩溃，请增大缓冲区。因为是跳转到比缓冲区还大的位置，文件中有，但是缓冲区没有读全。		//在这里，如果读不出来，请上bootmain.c中改一下读的页数～
//    u8 page_buf[page_size]; //4096字节的页缓冲区
    u8 *page_buf = (u8 *) ( 0x10000 + VERTUAL_MEM );

    //load Section Header (if it is relocatable / executable file)
    //文件处理每次都要从文件中读入，效率较低。但是为了可读性和理解，不使用缓冲区。
    //set up sec_hdr(s) => 数组
    struct sec_t sec_hdr[elf_hdr->e_shnum];  //注意第一个是NULL Section Header！！
    sec_ptr = &sec_hdr[0];
    sec_num = elf_hdr->e_shnum;
    long offset = elf_hdr->e_shoff;
    for(int i = 0; i < elf_hdr->e_shnum; i ++){
        small_fgets((char *)&sec_hdr[i], offset, elf_hdr->e_shentsize, (char *)page_buf);
//        printf("~~0x%08x\n", sec_hdr[i].sh_name);   //打印出name
//        printf("0x%08x\n", sec_hdr[i].sh_offset);   //打印出offset
        offset += elf_hdr->e_shentsize;      //get 下一张Section Table表
    }

    //load Program Header (only if it is executable file)
    //load prog_hdr(s)
    struct prog_t prog_hdr[elf_hdr->e_phnum];
    prog_ptr = &prog_hdr[0];
    prog_num = elf_hdr->e_phnum;
    long prog_offset = elf_hdr->e_phoff;
    if(elf_hdr->e_phoff != 0)        //只有是executable file才可以执行这里。如果是relocatable file, 由于没有链接，因此elf_hdr->phoff是0. 也就是还没有地址。
        for(int i = 0; i < elf_hdr->e_phnum; i ++){
            small_fgets((char *)&prog_hdr[i], prog_offset, elf_hdr->e_phentsize, (char *)page_buf);
            //      printf("~~0x%08x\n", prog_hdr[i].sh_name);  //打印出name
            //      printf("0x%08x\n", sec_hdr[i].sh_offset);   //打印出offset
            prog_offset += elf_hdr->e_phentsize;     //get 下一张Section Table表
        }

    //先获得这张表里边的sh_name偏移量，用来计算每个sh_name标记的真正名字，到shstrtab里边去查。
    u32 shstrtab_offset, shstrtab_size;
    shstrtab_offset = sec_hdr[elf_hdr->e_shstrndx].sh_offset;
    shstrtab_size = sec_hdr[elf_hdr->e_shstrndx].sh_size;


    for(int i = 1; i < elf_hdr->e_shnum; i ++){
        if(strcmp(".symtab", (const char *)(page_buf + shstrtab_offset + sec_hdr[i].sh_name)) == 0){
            symtab_offset = sec_hdr[i].sh_offset;
            symtab_size = sec_hdr[i].sh_size;
        }
        if(strcmp(".strtab", (const char *)(page_buf + shstrtab_offset + sec_hdr[i].sh_name)) == 0){
            strtab_offset = sec_hdr[i].sh_offset;
            strtab_size = sec_hdr[i].sh_size;
        }
    }
//    printf("0x%08x, 0x%08x\n", symtab_offset, strtab_offset);



    //读取symtab段  //获取所有函数在link之后的内存地址  //于是可以进行backtrace~
    //load symtab_hdr(s)
    symtable_num = symtab_size/sizeof(struct symtable_t);
    struct symtable_t symtab_hdr[symtab_size/sizeof(struct symtable_t)];
    symtable_ptr = &symtab_hdr[0];
    small_fgets((char *)&symtab_hdr, symtab_offset, symtab_size, (char *)page_buf);
    for(int i = 0; i < symtab_size/sizeof(struct symtable_t); i ++){
//        printf("0x%08x\n", symtab_hdr[i].st_value);
    }

    //读取strtab段  //获取所有文件名以及函数名以及全局变量名(printf有可能被优化成puts)
    //load strtab
    char strtab[strtab_size];
    strtab_ptr = strtab;
    small_fgets(strtab, strtab_offset, strtab_size, (char *)page_buf);
    for(int i = 0; i < strtab_size; i ++){
        printf("%c", strtab[i]);
        if(strtab[i] == 0)  printf(" ");
    }
//    printf("\n");
}

//得到处于某个地址的addr值 (仅适用于executable file， 因为已经重定向过了)
const char *get_func_name(u32 addr)
{
    for (int i = 0; i < symtable_num; i ++) {
        if(addr >= symtable_ptr[i].st_value && addr < symtable_ptr[i].st_value + symtable_ptr[i].st_size){
            return (const char *)(strtab_ptr + symtable_ptr[i].st_name);
        }
    }
    return NULL;
}

/**
 *  进入堆栈调用之前，一定回先做pushl ebp; mov esp, ebp; 也就是将ebp保存，然后把esp给了ebp，也即是，现在的ebp寄存器，正在指向自己原来的值了。
 *  紧接着调用函数，使用call指令。这时call指令会隐式地将跳转的函数地址压入栈中。这样，(u32)ebp寄存器+1就是跳转的函数的指针了。
 *  http://blog.csdn.net/chance_yin/article/details/21191285
 */
void print_backtrace()
{
    //  u32 *ebp;   //绝不可以定义成指针！！否则会出错！！！TAT！！！！！这汇编检查为啥这么严格。。
    u32 temp_ebp;       //明明是个指针。。竟然要定义成一个整型。。

    asm ("movl %%ebp, %0;"
            :"=r"(temp_ebp)
        );

    u32 *ebp = (u32 *)temp_ebp;
    u32 *return_addr = ebp + 1; //上一个函数的地址。索引到高地址的位置，就会得到跳转到上一个函数了。

    while(ebp)
    {
        const char *func_name;
        if((func_name = get_func_name(*return_addr)) != NULL && *return_addr >= VERTUAL_MEM)	//最后这里有些补丁。
            printf("%x, %s\n", *return_addr, get_func_name(*return_addr)); //此函数不要，直接索引到print_backtrace()外边的函数
        ebp = (u32 *)*ebp;
        return_addr = ebp + 1;
    }

}

void panic(const char *msg)
{
	printf("====================\n");
	printf("%s\n", msg);
	print_backtrace();
	printf("====================\n");
	while(1);
}
