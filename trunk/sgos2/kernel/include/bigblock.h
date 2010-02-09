#ifndef _BIGBLOCK_H_
#define _BIGBLOCK_H_

//类型定义
//typedef unsigned long size_t;
#include <sgos.h>
#include <ipc.h>

#define FREE_NODE 0x00000001

#define MAX_HASH_ENTRY 15

typedef struct bnode{
	//邻接链表
	struct bnode* prev;
	struct bnode* next;
	//Hash table
	struct bnode* hash_prev;
	struct bnode* hash_next;
	//分配描述
	size_t attribute;
	size_t size; //不包括分配描述符大小
	size_t addr; //内存地址
}bnode_t;

typedef struct bigblock{
	struct bnode*	first_node;	//used for debugging.
	struct bnode*	free_table[MAX_HASH_ENTRY];
	KSemaphore	semaphore;
}bigblock_t;


//导出函数
void*	bb_alloc(bigblock_t*, size_t);
void*	bb_alloc_ex(bigblock_t* who, size_t addr, size_t siz );
size_t	bb_free(bigblock_t*, void*);
void*	bb_calloc(bigblock_t*, size_t, size_t);
void* 	bb_realloc(bigblock_t*, void*, size_t);
void	bb_init_block(bigblock_t* who, size_t addr, size_t size);
void	bb_print_block(bigblock_t* who);
void	bb_free_all(bigblock_t * who);
int	bb_check_allocated(bigblock_t* who, size_t addr );

#endif //_BIGBLOCK_H_
