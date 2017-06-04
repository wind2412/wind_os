/*
 * malloc.c
 *
 *  Created on: 2017年5月28日
 *      Author: zhengxiaolin
 */

#include <malloc.h>

struct list_node chunk_head;

void malloc_init()
{
	list_init(&chunk_head);
}

//不连续的malloc。
void *malloc(u32 size)
{
	u32 total_alloc_size = sizeof(struct Chunk) + size;
	struct list_node *node = &chunk_head;
	while(node->next != &chunk_head){
		struct Chunk *chunk = GET_OUTER_STRUCT_PTR(node->next, struct Chunk, node);
		if(chunk->allocated == 0 && chunk->size >= total_alloc_size){		//split
			//对"下一个chunk"进行设置	但是这个chunk被使用的情况只有: size够大 + 目前的node是chunk_head之前的最后一个，而且不符合要求，即(chunk->allocated==1)或者(==0但是size不够大)。
			struct Chunk *next = (struct Chunk *)((u32)chunk + total_alloc_size);
			next->allocated = 0;
			next->size = chunk->size - total_alloc_size;		//要减去分割开的上一部分还有这个next的大小
			list_insert_after(&chunk->node, &next->node);		//不要插入！否则如果malloc之后free了，这个空闲内存还会被重设一次，然后重新插入一次.....
																//不存在这样的问题。因为本来的被分裂块会直接进行修改，新增只是增加分裂剩下的remain块。

			//对"此chunk"进行设置
			chunk->allocated = 1;
			chunk->size = total_alloc_size - sizeof(struct Chunk);

			return (void *)((u32)chunk + sizeof(struct Chunk));
//		}else if(chunk->node.next == &chunk_head){	//是最后一个的话，就查看“剩下的”能不能符合要求。
//			struct Chunk *little_remain = (struct Chunk *)((u32)chunk + sizeof(struct Chunk) + chunk->size);
//			if((u32)little_remain + sizeof(struct Chunk) + size < ROUNDUP((u32)little_remain)){	//够大
//				little_remain->size = size;
//				little_remain->allocated = 1;
//				list_insert_before(&chunk_head, &little_remain->node);
//				return (void *)((u32)little_remain + sizeof(struct Chunk));
//			}else{
//				goto fail;		//此生终于在正式场合里用上了goto！！哈哈哈哈哈哈！！
//			}
		}else{	//继续遍历还有哪个页能容纳下所需要的size
	fail:	if(chunk->node.next == &chunk_head)	break;	//这一句是防止如果现在是最后一个块，下一个是chunk_head，但是自身却少于size大小，如果是这样的话，else里会->next变成node自身，就会无限循环。。。恶心的设计。
			node = node->next;
			continue;
		}
	}

	//如果走到这里，那么就必须要申请新的页了。这样，势必要插入到list的最后一项。
	int need_page_num = total_alloc_size / PAGE_SIZE;
	if(total_alloc_size % PAGE_SIZE > 0)	need_page_num += 1;	//如果有余，加一页。
	struct Page *pages = alloc_page(need_page_num);
	struct Chunk *new_chunk = (struct Chunk *)pg_to_addr_la(pages);
	new_chunk->allocated = 1;
	new_chunk->size = size;
	list_insert_before(&chunk_head, &new_chunk->node);	//插入到队尾。

	u32 remain_mem = need_page_num * PAGE_SIZE - total_alloc_size;		//看看能不能split
	if(remain_mem > sizeof(struct Chunk)){		//如果还有一个Chunk的空：
		struct Chunk *remain_chunk = (struct Chunk *)((u32)new_chunk + total_alloc_size);
		remain_chunk->allocated = 0;
		remain_chunk->size = remain_mem - sizeof(struct Chunk);
		list_insert_before(&chunk_head, &remain_chunk->node);	//理由同上......
	}

	return (void *)((u32)new_chunk + sizeof(struct Chunk));
}

void free(void *addr)
{
	struct Chunk *chunk = (struct Chunk *)((u32)addr - sizeof(struct Chunk));
	chunk->allocated = 0;
	u32 this_size = chunk->size;

	//和上下块合并
	struct Chunk *prev = chunk->node.prev == &chunk_head ? NULL : GET_OUTER_STRUCT_PTR(chunk->node.prev, struct Chunk, node);
	struct Chunk *next = chunk->node.next == &chunk_head ? NULL : GET_OUTER_STRUCT_PTR(chunk->node.next, struct Chunk, node);

	if(prev != NULL && prev->allocated == 0 && (u32)prev + sizeof(struct Chunk) + prev->size == (u32)chunk){		//必须要保证是连续的内存！！
		list_delete(&chunk->node);
		prev->size += (this_size + sizeof(struct Chunk));
		chunk = prev;		//把chunk设为prev。 也就是，在和next合并之前，chunk更新为next前边的块指针。(因为，chunk本身可能被prev合并。)
	}

	if(next != NULL && next->allocated == 0 && (u32)chunk + sizeof(struct Chunk) + chunk->size == (u32)next){		//理由同上
		list_delete(&next->node);
		chunk->size += (next->size + sizeof(struct Chunk));
	}

	//由于vmm啥的init全是malloc初始化的。因此原来的[如果全free的话，就把所有page全都free了]的策略是错误的。因为根本就不可能全free了。
//	if((u32)chunk == ROUNDDOWN((u32)chunk) && (sizeof(struct Chunk) + chunk->size) % PAGE_SIZE == 0){	//现在的chunk占据了一个整页，删了它吧
//		list_delete(&chunk->node);
//		free_page(la_addr_to_pg((u32)chunk),  (sizeof(struct Chunk) + chunk->size) / PAGE_SIZE);
//	}
}


void test_malloc()
{
	void *addr1, *addr2, *addr3, *addr4, *addr5;
	printf("malloc addr is: %x\n", addr1 = malloc(1000));
	printf("malloc addr is: %x\n", addr2 = malloc(1000));
	printf("malloc addr is: %x\n", addr3 = malloc(5000));
	printf("malloc addr is: %x\n", addr4 = malloc(5000));
	printf("malloc addr is: %x\n", addr5 = malloc(5000));

	free(addr1);
	free(addr2);
	free(addr3);
	free(addr4);
	free(addr5);

	printf("after free:\n");

	printf("malloc addr is: %x\n", addr1 = malloc(1000));
	printf("malloc addr is: %x\n", addr2 = malloc(1000));
	printf("malloc addr is: %x\n", addr3 = malloc(5000));
	printf("malloc addr is: %x\n", addr4 = malloc(5000));
	printf("malloc addr is: %x\n", addr5 = malloc(5000));

	free(addr1);
	free(addr2);
	free(addr3);
	free(addr4);
	free(addr5);

	printf("after free:\n");

	printf("malloc addr is: %x\n", addr1 = malloc(1000));
	printf("malloc addr is: %x\n", addr2 = malloc(1000));
	printf("malloc addr is: %x\n", addr3 = malloc(5000));
	printf("malloc addr is: %x\n", addr4 = malloc(5000));
	printf("malloc addr is: %x\n", addr5 = malloc(5000));
}


