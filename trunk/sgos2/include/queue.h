#ifndef _QUEUE_H
#define _QUEUE_H


#define QUEUE_NAME_LEN	16

typedef int (*queue_search_func)(const void *, const void *);
typedef void (*queue_delete_func)(const void *);

typedef struct QUEUE_NODE{
	struct QUEUE_NODE	*pre, *next;
	void*			v;
}QUEUE_NODE, qnode_t;

typedef struct _QUEUE{
	void*		semaphore;	//这里用spin_lock好不?
	int		cur_num;	//当前链表元素个数
	int		max_num;	//最大允许元素个数
	QUEUE_NODE	*front;		//头节点
	QUEUE_NODE	*back;		//尾节点
	char		name[QUEUE_NAME_LEN];	//used for debugging.
	queue_delete_func del_func;
	char		use_sem;
}queue_t;

#define QUEUE_UNINTERRUPTABLE	1

EXTERN int queue_create( queue_t* l, int size, queue_delete_func del, const char* name, int use_sem );
EXTERN void* queue_pop_back( queue_t* l );
EXTERN void* queue_pop_front( queue_t* l );
EXTERN int queue_push_front( queue_t* l, void* data );
EXTERN int queue_push_back( queue_t* l, void* data );
EXTERN void* queue_search( queue_t* l, void*, queue_search_func search, qnode_t** ret_nod );
EXTERN void* queue_quick_search( queue_t* l, void*, qnode_t** ret_nod );
EXTERN void queue_cleanup( queue_t* l );
EXTERN int queue_is_empty( queue_t* l );
EXTERN void queue_remove( queue_t* l, QUEUE_NODE* data );

#endif
