#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sgos.h>
#include <thread.h>
#include <mutex.h>
#include <allocator.h>
#include <message.h>

#define MAX_PROCESS_NUM	1024
#define PROCESS_NAME_LEN	32

#define IS_KERNEL_PROCESS( p ) (p->user)


// 物理内存使用信息
typedef struct PAGE_INFO{
	struct PAGE_INFO*	next;
	uint			phys_addr;	//物理内存地址
	time_t			ctime;		//create time.
}PAGE_INFO;

// 进程内存信息
typedef struct MEMORY_INFO{
	struct PAGE_INFO*	page;		//使用页面链表
	uint			total_pages;	//页面计数
	uint			umem_size;	//用户空间分配大小
	uint			kmem_size;	//内核空间占用大小
	allocator_t		umem_mgr;	//用户空间分配管理器
	uint			max_umem;	//用户空间允许使用内存总大小
	uint			max_kmem;	//最大内核占用大小
}MEMORY_INFO;

// 内核进程结构体。
typedef struct PROCESS{
	uint				id;
	mutex_t				mutex;
	uint				user;
	struct PROCESS*			pre, *next;
	struct PROCESS*			parent, *child;
	struct THREAD*			thread;
	struct MEMORY_INFO		memory_info;
	struct MESSAGE_QUEUE		message;
	char				name[PROCESS_NAME_LEN];
	void*				information;
	uint				page_dir;	//page_dir
	uint				exit_code;
	void*				module;		//module informations
	void*				environment;	//process startup environment
	
}PROCESS;

void process_init();
PROCESS* current_proc();

#endif
