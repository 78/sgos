#ifndef _MESSAGE_H_
#define _MESSAGE_H_

typedef MSG_UNIT{
	void*				context;	//消息正文
	struct MSG_UNIT*	next;		//下一个消息
	struct MSG_UNIT*	pre;		//上一个消息
}MESSAGE_UNIT, MSG_UNIT;

typedef MESSAGE_QUEUE{
	void*		head;		//队列头
	void*		tail;		//队列尾
	uint		count;		//消息计数
	uint		total_size;	//数据空间大小
	uint		max_size;	//最大队列数据空间大小
	mutex_t		mutex;		//睡眠锁
}MESSAGE_QUEUE, MSG_QUEUE;


#endif
