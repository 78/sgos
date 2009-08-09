#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <sgos.h>
#include <mutex.h>

typedef struct MESSAGE{
	void*			context;	//消息正文
	struct MESSAGE*		next;		//下一个消息
	struct MESSAGE*		pre;		//上一个消息
	uint			size;		//消息占用空间
}MESSAGE, MSG;

typedef struct MESSAGE_QUEUE{
	MESSAGE*	head;		//队列头
	MESSAGE*	tail;		//队列尾
	uint		count;		//消息计数
	uint		total_size;	//数据空间大小
	uint		max_size;	//最大队列数据空间大小
	mutex_t		mutex;		//睡眠锁
}MESSAGE_QUEUE, MSG_QUEUE;


#endif
