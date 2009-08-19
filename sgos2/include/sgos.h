//sgos.h
/*
 *  SGOS Definitions
 *
 */

#ifndef __SGOS__H__
#define __SGOS__H__

#include <types.h>

#define PROCESS_NAME_LEN	128

#define RTC_FREQUENCY	1000	//1000Hz  这是时钟频率，具体在arch/i386/clock/rtc.c
#ifndef NULL
#define NULL		((void*)0)	//
#endif
#define FILE_NAME_LEN	128

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
}THREAD_INFO;

#endif //__SGOS__H__
