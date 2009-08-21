#ifndef _QUEUE_H
#define _QUEUE_H


#include <semaphore.h>

#define QUEUE_NAME_LEN	16

typedef int (*queue_search_func)(const void *, const void *);
typedef void (*queue_delete_func)(const void *);

typedef struct _QUEUE{
	sema_t		semaphore;	//这里用spin_lock好
	int		size;
	int		head;
	int		tail;
	void**		items;		//使用数组进行保存
	queue_delete_func	del_func;	//溢出时，是否删除
	char		name[QUEUE_NAME_LEN];	//used for debugging.
}queue_t;

int queue_create( queue_t* l, int size, queue_delete_func del, char* name );
void* queue_pop_from_tail( queue_t* l );
void* queue_pop_from_head( queue_t* l );
int queue_push_to_head( queue_t* l, void* data );
int queue_push_to_tail( queue_t* l, void* data );
void* queue_search( queue_t* l, void*, queue_search_func search );
void queue_cleanup( queue_t* l );
int queue_is_empty( queue_t* l );
void queue_remove( queue_t* l, void* data );

#endif
