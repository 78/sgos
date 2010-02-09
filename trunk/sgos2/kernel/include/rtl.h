#ifndef _RTL_H
#define _RTL_H

// Bits Functioins
void * RtlCopyMemory(void * dest, const void * src, int n);
void * RtlCopyMemory16(void * dest, const void * src, int n);
void * RtlCopyMemory32(void * dest, const void * src, int n);
void * RtlZeroMemory(void * s, int count);
void * RtlZeroMemory16(void * s, int count);
void * RtlZeroMemory32(void * s, int count);

#define QUEUE_NAME_LEN	16
typedef int (*KQueueSearcher)(const void *, const void *);
typedef void (*KQueueEraser)(const void *);
typedef struct KQueueNode{
	struct KQueueNode	*prev, *next;
	void*			v;
}KQueueNode;
typedef struct KQueue{
	void*		semaphore;	//这里用spin_lock好不?
	int		cur_num;	//当前链表元素个数
	int		max_num;	//最大允许元素个数
	KQueueNode	*front;		//头节点
	KQueueNode	*back;		//尾节点
	char		name[QUEUE_NAME_LEN];	//used for debugging.
	KQueueEraser del_func;
	char		use_sem;
}KQueue;

#define QUEUE_UNINTERRUPTABLE	1

int RtlCreateQueue( KQueue* l, int size, KQueueEraser del, const char* name, int use_sem );
void* RtlPopBackQueue( KQueue* l );
void* RtlPopFrontQueue( KQueue* l );
int RtlPushFrontQueue( KQueue* l, void* data );
int RtlPushBackQueue( KQueue* l, void* data );
void* RtlSearchQueue( KQueue* l, void*, KQueueSearcher search, KQueueNode** ret_nod );
void* RtlQuickSearchQueue( KQueue* l, void*, KQueueNode** ret_nod );
void RtlDestroyQueue( KQueue* l );
int RtlIsEmptyQueue( KQueue* l );
void RtlRemoveQueueElement( KQueue* l, KQueueNode* data );

#endif
