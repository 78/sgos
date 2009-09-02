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
#include <namespace.h>
#include <semaphore.h>
#include <string.h>

#define IS_THREAD( thr ) ( ((THREAD*)thr)->magic == THREAD_MAGIC )
#define IS_MODULE( mod ) ( ((MODULE*)mod)->magic == MODULE_MAGIC )
#define IS_PROCESS( proc ) (((PROCESS*)proc)->magic == PROCESS_MAGIC )
#define IS_WRITABLE( proc, addr, size ) ( ucheck_allocated( proc, (uint)addr ) )

#define SYSCALL0(id, type, name) static type sys_##name()
#define SYSCALL1(id, type, name, atype, a) static type sys_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) static type sys_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) static type sys_##name( atype a, btype b, ctype c )
#define SYSCALL4(id, type, name, atype, a, btype, b, ctype, c, dtype, d) static type sys_##name( atype a, btype b, ctype c, dtype d )

#include <apidef.h>

void* syscall_table[] = {
	//0-4
	sys_test,
	sys_dprint,
	sys_send,
	sys_recv,
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
	sys_thread_kill,
	//15-19
	sys_thread_set_priority,
	sys_thread_get_priority, 
	sys_thread_semget,
	sys_thread_semop,
	sys_thread_semctl,
	//20-24
	sys_process_create,
	sys_process_kill,
	sys_process_suspend,
	sys_process_resume,
	sys_process_self,
	//25-29
	sys_loader_open,
	sys_loader_close,
	sys_loader_symbol,
	sys_namespace_create,
	sys_namespace_delete,
	//30-34
	sys_namespace_match,
	NULL,
	NULL,
	NULL,
	NULL,
	//35-39
	sys_iomap_get,
	sys_iomap_set,
	sys_irq_register,
	sys_irq_unregister,
	sys_vm_map,
	//40-44
	sys_bios_call,
};
 

//返回计数
uint sys_test()
{
	static unsigned counter=0; 
	return counter++;
}

//输出调试信息
int sys_dprint( const char* buf )
{
	return debug_print( (char*)buf );
}
 
//发送消息
int sys_send( void* session, void* content, size_t len, uint flag )
{
	THREAD* dest;
	if( !IS_USER_MEMORY((uint)content) || !IS_USER_MEMORY((uint)content+len) )
		return -ERR_WRONGARG;
	dest = (THREAD*)((session_t*)session)->thread;
	if( !IS_THREAD(dest) )
		return -ERR_WRONGARG;
	return message_send( session, content, len, flag );
}

//接收消息
int sys_recv( void* session, void* content, size_t* len, uint flag )
{
	THREAD* dest;
	PROCESS* proc;
	proc = current_proc();
	if( !IS_WRITABLE(proc, session, sizeof(session_t)) ||
		!IS_WRITABLE(proc, content, *len) || 
		!IS_WRITABLE(proc, len, sizeof(size_t)) ){
		return -ERR_WRONGARG;
	}
	dest = (THREAD*)((session_t*)session)->thread;
	if( dest && !IS_THREAD(dest) )
		return -ERR_WRONGARG;
	return message_recv( session, content, len, flag );
}

//分配用户内存
void* sys_virtual_alloc( size_t siz, size_t addr, uint flag )
{
	void* ptr;
	if( flag & ALLOC_WITH_ADDR ){
		ptr = umalloc_ex( current_proc(), addr, siz );
	}else{
		ptr = umalloc( current_proc(), siz );
	}
	return ptr;
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
		//terminate the process
		kprintf("Program %s exited with code 0x%X\n", current_proc()->name, code );
	}
	thread_kill( thr, code );
	//process_kill
}

//创建线程
int sys_thread_create( size_t addr, uint* ret )
{
	THREAD* thr;
	if( IS_USER_MEMORY(addr) &&
		IS_WRITABLE( current_proc(), ret, sizeof(uint)) ){
		thr = thread_create( current_proc(), addr, 0 );
		if( thr ){
			*ret = (uint)thr;
			return 0;
		}
		return -ERR_UNKNOWN;
	}
	return -ERR_WRONGARG;
}

//返回当前线程ID
uint sys_thread_self()
{
	return (uint)current_thread();
}

//脱离线程
int sys_thread_detach( uint thread )
{
	PERROR("##not implemented.");
	return -ERR_NOIMP;
}

//等待线程结束
int sys_thread_join( uint thread, int* code )
{
	PERROR("##not implemented.");
	return -ERR_NOIMP;
}

//线程睡眠一段时间
int sys_thread_wait( time_t ms )
{
	thread_wait( ms );
	return 0;
}

//挂起线程
int sys_thread_suspend( uint thread )
{
	THREAD* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	thread_suspend( thr );
	return 0;
}

//启动线程
int sys_thread_resume( uint thread )
{
	THREAD* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	thread_resume( thr );
	return 0;
}

//结束线程
int sys_thread_kill( uint thread, int code )
{
	THREAD* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	thread_kill( thr, code );
	return 0;
}

//设置线程优先级
int sys_thread_set_priority( uint thread, int pri )
{
	THREAD* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	if( thr->process != current_proc() )
		return -ERR_LOWPRI;
	if( pri<1 || pri>4 )
		return -ERR_WRONGARG;
	if( pri == PRI_REALTIME )
		thr->process->realtime_thread = thr;
	else{
		if( thr->priority == PRI_REALTIME )
			thr->process->realtime_thread = NULL;
	}
	thr->priority = pri;
	return 0;
}

//获取线程优先级
int sys_thread_get_priority( uint thread, int* pri )
{
	THREAD* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	return thr->priority;
}

//返回一个空闲的信号量
int sys_thread_semget(int value)
{
	PROCESS* proc;
	int i;
	proc = current_proc();
	down( &proc->semaphore );
	for(i=0; i<MAX_SEM_NUM; i++ )
		if( proc->sem_array[i]==NULL ){
			proc->sem_array[i] = kmalloc(sizeof(sema_t));
			up( &proc->semaphore );
			sema_init_ex( proc->sem_array[i], value );
			return i;
		}
	up( &proc->semaphore );
	return -ERR_NOMEM;
}

int sys_thread_semop( int i, int op )
{
	PROCESS* proc;
	proc = current_proc();
	if( i<0 || i>=MAX_SEM_NUM || !proc->sem_array[i] )
		return -ERR_WRONGARG;
	switch( op ){
	case SEMOP_DOWN:
		sema_down( proc->sem_array[i] );
		return 0;
	case SEMOP_UP:
		sema_up( proc->sem_array[i] );
		return 0;
	case SEMOP_TRYDOWN:
		return sema_trydown( proc->sem_array[i] );
	}
	return -ERR_WRONGARG;
}

int sys_thread_semctl( int i, int cmd )
{
	PROCESS* proc;
	proc = current_proc();
	if( i<0 || i>=MAX_SEM_NUM || !proc->sem_array[i] )
		return -ERR_WRONGARG;
	switch( cmd ){
	case SEMCTL_FREE:
		sema_destroy( proc->sem_array[i] );
		kfree( proc->sem_array[i] );
		proc->sem_array[i] = NULL;
		break;
	}
	return -ERR_WRONGARG;
}

//创建进程
int sys_process_create( const char* cmdline, const char** var, void* cinfo, uint* retp )
{
	PROCESS* curproc, *newproc;
	THREAD* init;
	int len;
	env_t* env;
	curproc = current_proc();
	if( !cmdline || !IS_WRITABLE( curproc, retp, sizeof(uint)) )
		return -ERR_WRONGARG;
	//如果cmdline指向错误路径，则异常退出
	env = (env_t*)kmalloc( sizeof(env_t) );
	if( !env )
		return -ERR_NOMEM;
	memset( env, 0, sizeof(env_t) );
	//cmdline
	strncpy( env->cmdline, cmdline, PAGE_SIZE );
	//variables
	if( var ){
		char* p, **q, *end;
		end = env->variables + ENV_VARIABLES_SIZE;
		for( p=env->variables, q=(char**)var; *q && p<end; q++ ){
			len = strlen( *q )+1; //including '\0'
			if( p+len<end ){
				memcpy( p, *q, len );
				p+=len;
			}else{
				break;
			}
		}
	}
	//create process
	newproc = process_create( curproc );
	if( !newproc ){
		kfree( env );
		return -ERR_NOMEM;
	}
	//set environment kernel pointer
	newproc->environment = env;
	//proc/newproc.c
	extern void init_newproc();
	init = thread_create( newproc, (uint)init_newproc, KERNEL_THREAD );
	if( !init ){
		//failed???
		process_kill( curproc, 0 );
		return -ERR_NOMEM;
	}
	thread_resume( init );
	return 0;
}

//结束进程
int sys_process_kill( uint proc, int code )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//挂起进程
int sys_process_suspend( uint proc)
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//启动进程
int sys_process_resume( uint proc )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//当前进程ID
uint sys_process_self()
{
	return (uint)current_proc();
}

//加载器 返回加载id
int sys_loader_open( char* file, uint* ret_mod )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//卸载库
int sys_loader_close( uint mod )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//获得过程
size_t sys_loader_symbol( uint mod, char* name )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//命名空间
int sys_namespace_create( uint thread, char* name )
{
	THREAD* thr;
	int ret;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	if( thr->process != current_proc() )
		return -ERR_LOWPRI;
	ret = name_insert( thr, name );
	return ret;
}

int sys_namespace_delete( uint thread, char* name )
{
	THREAD* thr;
	int ret;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (THREAD*)thread;
	if( thr->process != current_proc() )
		return -ERR_LOWPRI;
	ret = name_remove( thr, name );
	return ret;
}

uint sys_namespace_match( char* name )
{
	uint thread;
	thread = (uint)name_match( name );
	return thread;
}

//返回io位图
int sys_iomap_get( uchar* buf, size_t buf_size )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//设置io位图 
int sys_iomap_set( uchar* buf, size_t buf_len  )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//注册irq消息
int sys_irq_register( int tid, int irq )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

int sys_irq_unregister( int tid, int irq )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

// 进程虚拟内存映射到物理地址
int sys_vm_map( size_t vaddr, size_t paddr, size_t map_size, uint flag )
{
	//检查参数是否页对齐
	if( vaddr%PAGE_SIZE || paddr%PAGE_SIZE || map_size%PAGE_SIZE )
		return -ERR_WRONGARG;
	//不允许映射内核空间
	if( !IS_USER_MEMORY(vaddr) || !IS_USER_MEMORY(vaddr+map_size-1) )
		return -ERR_WRONGARG;
	if( current_thread()->process->uid != ADMIN_USER )
		return -ERR_LOWPRI;
	if( flag & MAP_UNMAP ){
		unmap_pages( vaddr, paddr, map_size );
	}
	if( flag & MAP_READONLY )
		map_pages( current_proc()->page_dir, vaddr, paddr, map_size, P_USER );
	else
		map_pages( current_proc()->page_dir, vaddr, paddr, map_size, P_USER | P_WRITE );
	return 0;
}

//BIOS调用
int sys_bios_call( int interrupt, void* context, size_t siz )
{
	if( current_thread()->process->uid != ADMIN_USER )
		return -ERR_LOWPRI;
	if( !IS_WRITABLE( current_proc(), context, siz ) )
		return -ERR_WRONGARG;
	return bios_call( interrupt, context, siz );
}

