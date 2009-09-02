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
#define ADMIN_USER		0

#define PROCESS_NAME_LEN	128

#ifndef NULL
#define NULL			((void*)0)	//
#endif

#define FILE_NAME_LEN		128
#define PATH_LEN		256
#define NAME_LEN		64		//NameSpace

// 文件操作 (Copied from SGOS1)
#define FILE_READ 		1
#define FILE_WRITE 		2

#define FILE_FLAG_CREATE	1
#define FILE_FLAG_APPEND	2
#define FILE_FLAG_DELETE	4	//关闭文件时候自动删除

#define FILE_ATTR_RDONLY	1	//只读文件
#define FILE_ATTR_DIR		2	//目录文件
#define FILE_ATTR_SYSTEM	4	//系统文件，不允许用户直接访问。
#define FILE_ATTR_HIDDEN	8	//隐藏文件

//VFS (Copied from SGOS1)
//Control
#define VFS_SET_TYPE		1
#define VFS_SET_VALUE		2
//Type
#define VFS_TYPE_NULL 		0	//空文件
#define VFS_TYPE_FS		1	//子文件系统
#define VFS_TYPE_DEV		2	//设备文件
#define VFS_TYPE_DIR		3	//目录文件
#define VFS_TYPE_STRING		4	//字符串
#define VFS_TYPE_PIPE		5	//管道
#define VFS_TYPE_DEVSYS		6	//设备系统
//Flag
#define VFS_FLAG_TEMP		1	//临时文件
#define VFS_FLAG_KERNEL		2	//内核文件，用户进程禁止访问
#define VFS_FLAG_READ		4	//可读
#define VFS_FLAG_WRITE		8	//可写

#define SEEK_CUR		0
#define SEEK_SET		1
#define SEEK_END		2

//标准输出输入3个
#ifndef STDIN
#define STDIN 0
#endif
#ifndef STDOUT
#define STDOUT 1
#endif
#ifndef STDERR
#define STDERR 2
#endif

//目录搜索
typedef struct _DIRECTORY_ENTRY{
    char name[FILE_NAME_LEN];
    int size;
    int attr;
    time_t ctime;
    time_t mtime;
}DIRENTRY, dir_t;

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
	char*			cmdline;		//命令行
	char*			variables;		//环境变量
}PROCESS_INFO;

//线程信息块
typedef struct THREAD_INFO{
	void*			exception_list;		//00 set by user
	size_t			stack_base;		//04 堆栈地址
	size_t			stack_size;		//08 堆栈大小
	void*			other_info;		//0C 线程其它信息
	void*			process_info;		//10 进程信息块地址
	void*			unused2;		//14 保留
	struct THREAD_INFO*	self;			//18 指向线程信息块地址
	char*			environment;		//1C 线程环境信息
	int			pid;			//20 进程id
	int			tid;			//24 线程id
	void*			local_storage;		//28 线程局部变量存储地址
	int			errno;			//2C 错误号
	time_t			time;			//30 当前时间
	void*			messenger;		//34 线程消息投递员(没用)
	size_t			entry_address;		//38 线程用户态入口
}THREAD_INFO;

//消息机制会话信息
typedef struct _SESSION{
	/* 发送时候thread是目标线程，接收时候thread是发送者线程 */
	uint			thread;
	/* sequence目前未使用 */
	uint			sequence;
	/* process仅对接收者可见，在接收的时候，由内核设置。 */
	uint			process;
}session_t;

typedef struct THREAD_CONTEXT{
	t_32			gs, fs, es, ds;
	t_32			edi, esi, ebp, kesp, ebx, edx, ecx, eax;
	t_32			eip, cs, eflags, esp, ss;
}THREAD_CONTEXT;

//API 
#define	MSG_SEND_TO_ALL		0
#define MSG_PENDING		0x80000000
#define MSG_KEEP		0x40000000

#define ALLOC_WITH_ADDR		1

#define MAP_UNMAP		1
#define MAP_READONLY		2

//系统错误号
#define ERR_NOMEM		1	//No memory
#define ERR_NOIMP		2	//Not implemented
#define ERR_WRONGARG		3	//Wrong argument
#define	ERR_NONE		4	//No results
#define ERR_UNKNOWN		5	//Unknown error
#define ERR_LOWPRI		6	//Low privilege
#define ERR_NOINIT		7	//Not initialized
#define ERR_NODEST		8	//No destination or wrong destination
#define ERR_NOPATH		9	//Invalid path
#define ERR_INVALID		10	//Invalid device

//线程优先级
#define PRI_REALTIME		1	//单线程独占模式
#define PRI_HIGH		2	//高优先级
#define PRI_NORMAL		3	//一般优先级
#define PRI_LOW			4	//最低优先级

//信号灯操作
#define SEMOP_DOWN		1	//P操作
#define SEMOP_UP		2	//V操作
#define SEMOP_TRYDOWN		3

#define SEMCTL_FREE		1	//释放信号灯

#endif //__SGOS__H__
