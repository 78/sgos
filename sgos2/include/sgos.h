//sgos.h
/*
 *  SGOS Definitions
 *
 */

#ifndef __SGOS__H__
#define __SGOS__H__

#include <types.h>

#ifdef __cplusplus 
#define EXTERN extern "C" 
#else
#define EXTERN extern
#endif

// 系统账号
#define ADMIN_USER	0

#define PROCESS_NAME_LEN	128

#ifndef NULL
#define NULL		((void*)0)	//
#endif
#define FILE_NAME_LEN	128
#define NAME_LEN	64		//NameSpace

typedef unsigned int thread_t;

//进程信息块
typedef struct PROCESS_INFO{
	char			name[PROCESS_NAME_LEN];	//进程名称
	int			pid;			//进程id
	int			uid;			//用户id
	int			gid;			//用户组id
	int			mid;			//可执行模块id
	int			parent;			//父进程id
	size_t			entry_address;		//程序入口
	int			main_thread;		//主线程id
	void*			global_storage;		//进程变量存储地址
}PROCESS_INFO;

//线程信息块
typedef struct THREAD_INFO{
	void*			exception_list;		//00 set by user
	size_t			stack_base;		//04 堆栈地址
	size_t			stack_size;		//08 堆栈大小
	void*			other_information;	//0C 线程其它信息
	void*			process_info;		//10 进程信息块地址
	void*			unused2;		//14 保留
	struct THREAD_INFO*	self;			//18 指向线程信息块地址
	char*			environment;		//1C 线程环境信息
	int			pid;			//20 进程id
	int			tid;			//24 线程id
	void*			local_storage;		//28 线程局部变量存储地址
	int			errno;			//2C 错误号
	time_t			time;			//30 当前时间
	void*			messenger;		//34 线程消息投递员
}THREAD_INFO;

//消息机制会话信息
typedef struct _SESSION{
	uint	thread;
	uint	sequence;
}session_t;
#define	MSG_SEND_TO_ALL		0
#define MSG_PENDING		0x80000000
#define MSG_KEEP		0x40000000

//系统错误号
#define ERR_NOMEM	1	//No memory
#define ERR_NOIMP	2	//Not implemented
#define ERR_WRONGARG	3	//Wrong argument
#define	ERR_NONE	4	//No results
#define ERR_UNKNOWN	5	//Unknown error
#define ERR_LOWPRI	6	//Low privilege
#define ERR_NOINIT	7	//Not initialized
#define ERR_NODEST	8	//No destination or wrong destination

//线程优先级
#define PRI_REALTIME	1	//单线程独占模式
#define PRI_HIGH	2	//高优先级
#define PRI_NORMAL	3	//一般优先级
#define PRI_LOW		4	//最低优先级

#define SEMOP_DOWN	1	//P操作
#define SEMOP_UP	2	//V操作
#define SEMOP_TRYDOWN	3

#define SEMCTL_FREE	1	//释放信号灯

#endif //__SGOS__H__
