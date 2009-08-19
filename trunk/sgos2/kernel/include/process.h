#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sgos.h>
#include <thread.h>
#include <mutex.h>
#include <bigblock.h>
#include <message.h>
#include <module.h>

#define MAX_PROCESS_NUM	1024

#define IS_KERNEL_PROCESS( p ) (p->user)

// 进程运行用户态环境信息
typedef struct ENVIRONMENT{
	char	cmd_line[PAGE_SIZE];
	char	variables[PAGE_SIZE];
}ENVIRONMENT;

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
	int			umem_size;	//用户空间分配大小
	int			kmem_size;	//内核空间占用大小
	bigblock_t		umem_mgr;	//用户空间分配管理器
	int			max_umem;	//用户空间允许使用内存总大小
	int			max_kmem;	//最大内核占用大小
}MEMORY_INFO;

// 内核进程结构体。
typedef struct PROCESS{
	uint				pid;		//进程标识
	mutex_t				mutex;	
	uint				uid;		//用户
	struct PROCESS*			pre, *next;	//进程链表，兄弟关系
	struct PROCESS*			parent, *child;	//父子进程
	struct THREAD*			thread;		//第一个线程
	struct THREAD*			main_thread;	//主线程
	struct MEMORY_INFO		memory_info;	//内存信息
	struct MESSAGE_QUEUE		message;	//消息
	char				name[PROCESS_NAME_LEN];	//进程名称
	PROCESS_INFO*			process_info;		//用户空间信息
	uint				page_dir;	//page_dir
	uint				exit_code;	//退出码
	ENVIRONMENT*			environment;	//process startup environment
	MODULE_LINK*			module_link;	//module informations
	size_t				module_addr;	//这个设置了，表示模块数据的内存地址
}PROCESS;

//第一个进程初始化
void process_init();
//返回当前进程
PROCESS* current_proc();
//创建进程
PROCESS* process_create( PROCESS* parent, ENVIRONMENT* env );

#endif
