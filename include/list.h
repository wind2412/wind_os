/*
 * list.h
 *
 *  Created on: 2017年5月27日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_LIST_H_
#define INCLUDE_LIST_H_

//仿照linux的双向链表

struct list_node{
	struct list_node* prev;
	struct list_node* next;
};

__attribute__((always_inline)) inline void list_init(struct list_node *node);

__attribute__((always_inline)) inline void list_insert_before(struct list_node *node, struct list_node *new_node);

__attribute__((always_inline)) inline void list_insert_after(struct list_node *node, struct list_node *new_node);

__attribute__((always_inline)) inline void list_delete(struct list_node *node);

#define get_outer_struct_ptr(node, type, member) ( (type *)( (u32)node - (u32)(&((type *)0)->member) ) )

/**********************************/

struct list_node head;

__attribute__((always_inline)) inline void list_init(struct list_node *node){
	node->prev = node;
	node->next = node;
}

__attribute__((always_inline)) inline void list_insert_before(struct list_node *node, struct list_node *new_node){
	new_node->prev = node->prev;
	node->prev->next = new_node;
	new_node->next = node;
	node->prev = new_node;
}

__attribute__((always_inline)) inline void list_insert_after(struct list_node *node, struct list_node *new_node){
	new_node->next = node->next;
	node->next->prev = new_node;
	new_node->prev = node;
	node->next = new_node;
}

__attribute__((always_inline)) inline void list_delete(struct list_node *node){
	//注意：因为并没有实现malloc和free，不要用。
	node->next->prev = node->prev;
	node->prev->next = node->next;
}





#endif /* INCLUDE_LIST_H_ */
