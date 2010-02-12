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

// units
#define	MB(a) (a<<20)
#define KB(a) (a<<10)

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

typedef unsigned int ThreadId_t;
typedef unsigned int SpaceId_t;
/*
//进程信息块
typedef struct ProcessInformation{
	char			ProcessName[PROCESS_NAME_LEN];	//进程名称
	int			ProcessId;			//进程id
	int			UserId;				//用户id
	int			GroupId;			//用户组id
	int			ModuleId;			//可执行模块id
	int			ParentProcessId;		//父进程id
	size_t			EntryAddress;			//程序入口
	int			MainThreadId;			//主线程id
	void*			GlobalStorage;			//进程变量存储地址
	char*			CommandLine;			//命令行
	char*			EnvironmentViriables;		//环境变量
}ProcessInformation;
*/

typedef struct SpaceInformation{
	
}SpaceInformation;

//SGOS2 线程信息块
typedef struct ThreadInformation{
	void*			ExceptionList;		//00 set by user
	size_t			StackBase;		//04 堆栈地址
	size_t			StackLimit;		//08 堆栈大小
	void*			OtherInformation;	//0C 线程其它信息
	void*			ProcessInformation;	//10 进程信息块地址
	void*			Unused;			//14 保留
	struct ThreadInformation*Self;			//18 指向线程信息块地址
	char*			Environment;		//1C 线程环境信息
	uint			ProcessId;		//20 进程id
	uint			ThreadId;		//24 线程id
	void*			LocalStorage;		//28 线程局部变量存储地址
	int			ErrorCode;		//2C 错误号
	time_t			CurrentTime;		//30 当前时间
	void*			Messenger;		//34 线程消息投递员(没用)
	size_t			EntryAddress;		//38 线程用户态入口
	void*			SpaceInformation;	//3C 地址空间信息
	uint			SpaceId;		//40 地址空间Id
}ThreadInformation;

typedef struct Message{
	uint	ThreadId;	//send to who
	time_t	Time;		//sendtime
	uint	Command;	//
	uint	Arguments[12];	//Parameters
	void*	Data;		//pointer of the data page
}Message;

// Intel x86 Cpu Registers
typedef struct ThreadContext{
	t_32			gs, fs, es, ds;
	t_32			edi, esi, ebp, kesp, ebx, edx, ecx, eax;
	t_32			eip, cs, eflags, esp, ss;
}ThreadContext;

//API 
#define	MSG_SEND_TO_ALL		0
#define MSG_KEEP		0x40000000

#define ALLOC_WITH_ADDR		1

#define MAP_ADDRESS		2
#define MAP_ATTRIBUTE		4
#define ALLOC_VIRTUAL		1

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
#define ERR_IO			11	//IO error
#define ERR_NOBUF		12	//Buffer too small

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
