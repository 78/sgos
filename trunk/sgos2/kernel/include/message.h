#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <sgos.h>
#include <semaphore.h>

//Give it a proper value!!
#define MAX_MESSAGES_IN_QUEUE	100

//内核消息描述符
//message structure in user mode
typedef struct KMESSAGE{
	session_t	session;	//会话信息
	void*		content;	//消息正文
	uint		length;		//消息正文长度
	void*		dest;		//接收者
	void*		source;		//发送者
	uint		flag;		//参数
}KMESSAGE;

//发送消息
//发送成功返回消息序号，发送失败返回负值
int	message_send( 
	session_t*	session, 	//会话信息
	void*		content, 	//消息正文
	size_t		len, 		//消息正文长度
	uint		flag		//发送参数
);

//接收消息
//接收成功返回消息长度，失败返回负值
//如果是因为缓冲区太小而失败，会修改消息缓冲区长度为恰当大小。
int	message_recv(
	session_t*	session,	//发送者信息
	void*		buf,		//消息正文缓冲区
	size_t*		siz,		//消息正文缓冲区长度
	uint		flag		//接收参数
);

// 线程消息初始化
void message_init( struct THREAD* thr );
// 释放消息队列占用的空间
void message_destroy( struct THREAD* thr );

#endif
