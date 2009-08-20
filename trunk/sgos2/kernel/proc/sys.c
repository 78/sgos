//System Calls

#include <sgos.h>
#include <arch.h>
#include <time.h>
#include <process.h>
#include <mm.h>
#include <thread.h>
#include <debug.h>
#include <loader.h>
#include <module.h>
#include <message.h>

#define SYSCALL0(id, type, name) static type sys_##name()
#define SYSCALL1(id, type, name, atype, a) static type sys_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) static type sys_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) static type sys_##name( atype a, btype b, ctype c )
#define SYSCALL4(id, type, name, atype, a, btype, b, ctype, c, dtype, d) static type sys_##name( atype a, btype b, ctype c, dtype d )

#include <apidef.h>

void* syscall_table[] = {
	//0-4
	sys_clock,
	sys_dprint,
	sys_msg_send,
	sys_msg_recv,
	sys_virtual_alloc,
	//5-9
	sys_virtual_free,
	sys_thread_exit,
	sys_thread_create,
	sys_thread_self,
	sys_thread_detach,
	//10-14
	sys_thread_join,
	sys_thread_wait,
	sys_thread_suspend,
	sys_thread_resume,
	sys_thread_terminate,
	//15-19
	sys_thread_set_priority,
	sys_thread_get_priority, 
	NULL,
	NULL,
	NULL,
	//20-24
	sys_process_create,
	sys_process_terminate,
	sys_process_suspend,
	sys_process_resume,
	sys_process_self,
	//25-29
	sys_loader_load,
	sys_loader_unload,
	sys_loader_get_proc,
	sys_namespace_register,
	sys_namespace_unregister,
	//30-34
	sys_iomap_get,
	sys_iomap_set,
	sys_irq_register,
	sys_irq_unregister,
	sys_virtual_map
};

//返回时钟计数
uint sys_clock()
{
	extern unsigned rtc_second;    //unit: s
	return rtc_second;
}

//输出调试信息
int sys_dprint( const char* buf )
{
	return debug_print( (char*)buf );
}
 
//发送消息
int sys_msg_send( const char* buf, size_t len )
{
	PERROR("not implemented.");
	return 0;
}

//接收消息
int sys_msg_recv( char* buf, size_t buf_siz, uint flag )
{
	PERROR("not implemented.");
	return 0;
}

//分配用户内存
void* sys_virtual_alloc( size_t siz )
{
	return umalloc( current_proc(), siz );
}

//释放内存
void sys_virtual_free( void* p )
{
	ufree( current_proc(), p );
}

//退出线程
void sys_thread_exit( int code )
{
	//直接终止
	THREAD* thr;
	thr = current_thread();
	if( current_proc()->main_thread == thr ){
		kprintf("Program %s exited with code 0x%X\n", current_proc()->name, code );
	}
	thread_terminate( thr, code );
}

//创建线程
int sys_thread_create( size_t addr )
{
	THREAD* thr;
	if( IS_USER_MEMORY(addr) ){
		thr = thread_create( current_proc(), addr );
		if( thr )
			return thr->tid;
	}
	return -1;
}

//返回当前线程ID
int sys_thread_self()
{
	return current_thread()->tid;
}

//脱离线程
int sys_thread_detach( int tid )
{
	PERROR("not implemented.");
	return 0;
}

//等待线程结束
int sys_thread_join( int tid, int* code )
{
	PERROR("not implemented.");
	return 0;
}

//线程睡眠一段时间
int sys_thread_wait( time_t ms )
{
	thread_wait( ms );
	return 0;
}

//挂起线程
int sys_thread_suspend( int tid )
{
	THREAD* thr;
	thr = thread_get( tid );
	if( thr ){
		thread_suspend( thr );
		return 0;
	}
	return -1;
}

//启动线程
int sys_thread_resume( int tid )
{
	THREAD* thr;
	thr = thread_get( tid );
	if( thr ){
		thread_resume( thr );
		return 0;
	}
	return -1;
}

//结束线程
int sys_thread_terminate( int tid, int code )
{
	THREAD* thr;
	thr = thread_get( tid );
	if( thr ){
		thread_terminate( thr, code );
		return 0;
	}
	return -1;
}

//设置线程优先级
int sys_thread_set_priority( int tid, int pri )
{
	PERROR("not implemented.");
	return 0;
}

//获取线程优先级
int sys_thread_get_priority( int tid, int* pri )
{
	PERROR("not implemented.");
	return 0;
}

//进程管理
int sys_process_create( char* file, void* environment, void* create_info )
{
	PERROR("not implemented.");
	return 0;
}

//结束进程
int sys_process_terminate( int id, int code )
{
	PERROR("not implemented.");
	return 0;
}

//挂起进程
int sys_process_suspend( int id )
{
	PERROR("not implemented.");
	return 0;
}

//启动进程
int sys_process_resume( int id )
{
	PERROR("not implemented.");
	return 0;
}

//当前进程ID
int sys_process_self()
{
	return current_proc()->pid;
}

//加载器 返回加载id
int sys_loader_load( char* file )
{
	PERROR("not implemented.");
	return 0;
}

//卸载库
int sys_loader_unload( int id )
{
	PERROR("not implemented.");
	return 0;
}

//获得过程
size_t sys_loader_get_proc( int id, char* name )
{
	PERROR("not implemented.");
	return 0;
}

//命名空间
int sys_namespace_register( int tid, char* name )
{
	PERROR("not implemented.");
	return 0;
}

int sys_namespace_unregister( int tid, char* name )
{
	PERROR("not implemented.");
	return 0;
}

//返回io位图
int sys_iomap_get( uchar* buf, size_t buf_size )
{
	PERROR("not implemented.");
	return 0;
}

//设置io位图 
int sys_iomap_set( uchar* buf, size_t buf_len  )
{
	PERROR("not implemented.");
	return 0;
}

//注册irq消息
int sys_irq_register( int tid, int irq )
{
	PERROR("not implemented.");
	return 0;
}

int sys_irq_unregister( int tid, int irq )
{
	PERROR("not implemented.");
	return 0;
}

int sys_virtual_map( size_t vaddr, size_t paddr, size_t map_size )
{
	PERROR("not implemented.");
	return 0;
}
