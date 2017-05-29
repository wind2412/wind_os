/*
 * malloc.c
 *
 *  Created on: 2017年5月28日
 *      Author: zhengxiaolin
 */

#ifndef MM_MALLOC_C_
#define MM_MALLOC_C_

#include <malloc.h>

struct list_node chunk_head;

u32 heap_max;		//malloc内存开始的地方

void malloc_init()
{
	extern u32 pt_begin;		//全局情况下，全局的extern变量不能赋给另一个全局变量。error：(extern variable) pt_begin is not constant.
	heap_max = pt_begin;
	chunk_head.next = &((struct Chunk *)heap_max)->node;
	chunk_head.prev = &((struct Chunk *)heap_max)->node;
	//设置第一块malloc(其实没有malloc，只是为了编程方便)
	struct Page first_page = alloc_page();	//先分出来一页。
	//设置这一页。
	((struct Chunk *)heap_max)->allocated = 0;
	((struct Chunk *)heap_max)->size = PAGE_SIZE;
	((struct Chunk *)heap_max)->node.next = &chunk_head;
	((struct Chunk *)heap_max)->node.prev = &chunk_head;
}

//first fit算法 hx_kern
void *malloc(u32 size)
{
	u32 total_alloc_size = sizeof(struct Chunk) + size;
	struct list_node *node = &chunk_head;	//最终发现按照Page的malloc实在是不现实的策略。遂改用hx的按照Chunk的malloc策略，先分配Chunk，再申请Page。有种先斩后奏的感觉。
	while(node->next != node){
		struct Chunk *chunk = GET_OUTER_STRUCT_PTR(node->next, struct Chunk, node);
		if(chunk->allocated == 0 && chunk->size >= total_alloc_size){		//split
			//对"下一个chunk"进行设置
			struct Chunk *next = (struct Chunk *)((u32)chunk + total_alloc_size);
			next->allocated = 0;
			next->size = chunk->size - total_alloc_size;		//要减去分割开的上一部分还有这个next的大小
			list_insert_after(&chunk->node, &next->node);

			//对"此chunk"进行设置
			chunk->allocated = 1;
			chunk->size = total_alloc_size;

			return (void *)((u32)chunk + sizeof(struct Chunk));
		}else{	//继续遍历还有哪个页能容纳下所需要的size
			if(chunk->node.next == &chunk_head)	break;	//这一句是防止如果现在是最后一个块，下一个是chunk_head，但是自身却少于size大小，如果是这样的话，else里会->next变成node自身，就会无限循环。。。恶心的设计。
			node = node->next;
			continue;
		}
	}

	//如果程序没有返回，而是走到这里，那就说明必须要从heap_max指向的位置开始分配页面了。
	struct Chunk *prev;
	if(GET_OUTER_STRUCT_PTR(chunk_head.prev, struct Chunk, node)->allocated == 1){
		prev = GET_OUTER_STRUCT_PTR(chunk_head.prev, struct Chunk, node);
	}else{
		prev = GET_OUTER_STRUCT_PTR(chunk_head.prev->prev, struct Chunk, node);
	}
	while(heap_max < (u32)prev + sizeof(struct Chunk) + prev->size + total_alloc_size){	//不断分配页面，如果size太大的话
		struct Page page = alloc_page();
		heap_max += PAGE_SIZE;		//更新下次如果前边malloc的没有free的话，想要再malloc的新chunk位置
	}
	struct Chunk *cur = (struct Chunk *)((u32)prev + sizeof(struct Chunk) + prev->size);
	list_insert_before(&chunk_head, &cur->node);
	cur->allocated = 1;
	cur->size = size;
	return (void *)((u32)cur + sizeof(struct Chunk));
}

void free(struct Chunk chunk)
{

}





#endif /* MM_MALLOC_C_ */
