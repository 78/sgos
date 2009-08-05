#ifndef _ALLOC_H_
#define _ALLOC_H_

//类型定义
//typedef unsigned long size_t;
#include <sgos.h>

#define FREE_NODE 0x00000001

#define MAX_HASH_ENTRY 15

typedef struct node{
	//邻接链表
	struct node* pre;
	struct node* next;
	//Hash table
	struct node* hash_pre;
	struct node* hash_next;
	//分配描述
	size_t attribute;
	size_t size; //不包括分配描述符大小
}node_t;

typedef struct allocator{
	struct node* first_node;	//used for debugging.
	struct node* free_table[MAX_HASH_ENTRY];
}allocator_t;

#define IS_FREE_NODE(nod) (nod->attribute&FREE_NODE )
#define MAKE_FREE(nod) (nod->attribute |= FREE_NODE)
#define MAKE_OCCUPIED(nod) (nod->attribute &= ~FREE_NODE )

//导出函数
void*	mm_alloc(allocator_t*, size_t);
void	mm_free(allocator_t*, void*);
void*	mm_calloc(allocator_t*, size_t, size_t);
void* 	mm_realloc(allocator_t*, void*, size_t);
void	mm_init_block(allocator_t* who, size_t addr, size_t size);
void	mm_print_block(allocator_t* who);
void	mm_free_all(allocator_t * who);

#endif //_ALLOC_H_
