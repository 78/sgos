#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sgos.h>
#include <thread.h>
#include <semaphore.h>
#include <bigblock.h>
#include <message.h>
#include <module.h>
#include <queue.h>

#define PROCESS_MAGIC	0xFF0B46FC

#define MAX_PROCESS_NUM	1024
#define MAX_SEM_NUM	64

#define IS_KERNEL_PROCESS( p ) (p->user)
#define ENV_VARIABLES_SIZE	PAGE_SIZE*4

// 进程运行用户态环境信息
typedef struct ENVIRONMENT{
	char	cmdline[PAGE_SIZE];
	char	variables[ENV_VARIABLES_SIZE];
}ENVIRONMENT, env_t;

// 物理内存使用信息
typedef struct PAGE_INFO{
	struct PAGE_INFO*	next;
	struct PAGE_INFO*	pre;
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
	int				pid;		//进程标识
	sema_t				semaphore;	
	uint				uid;		//用户
	uint				magic;		//进程标识
	struct PROCESS*			pre, *next;	//进程链表，兄弟关系
	struct PROCESS*			parent, *child;	//父子进程
	struct THREAD*			thread;		//第一个线程
	struct THREAD*			main_thread;	//主线程
	struct THREAD*			realtime_thread;	//实时线程
	struct MEMORY_INFO		memory_info;	//内存信息
	queue_t				message_queue;	//消息
	char				name[PROCESS_NAME_LEN];	//进程名称
	PROCESS_INFO*			process_info;		//用户空间信息
	uint				page_dir;	//page_dir
	uint				exit_code;	//退出码
	ENVIRONMENT*			environment;	//process startup environment
	MODULE_LINK*			module_link;	//module informations
	size_t				module_addr;	//这个设置了，表示模块数据的内存地址
	size_t				module_size;	//模块数据的内存大小
	sema_t*				sem_array[MAX_SEM_NUM];	//信号量集
	int				bios_mode;	//是否使用VM86模式
}PROCESS;

//第一个进程初始化
void process_init( const char* cmdline );
//返回当前进程
PROCESS* current_proc();
//创建进程
PROCESS* process_create( PROCESS* parent );
int process_kill( PROCESS* proc, int exit_code );
//由id获取进程
PROCESS* process_get( int pid );

#endif
