//System Calls

#include <sgos.h>
#include <arch.h>
#include <time.h>
#include <mm.h>
#include <tm.h>
#include <kd.h>
#include <ipc.h>


#define SYSCALL0(id, type, name) static type Api_##name()
#define SYSCALL1(id, type, name, atype, a) static type Api_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) static type Api_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) static type Api_##name( atype a, btype b, ctype c )
#define SYSCALL4(id, type, name, atype, a, btype, b, ctype, c, dtype, d) static type Api_##name( atype a, btype b, ctype c, dtype d )

#include <apidef.h>

void* SystemCallTable[] = {
	//0-4
	Api_Test,
	Api_Print,
	/*
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
	*/
};
 

//返回计数
uint Api_Test()
{
	static unsigned counter=0; 
	return counter++;
}

//输出调试信息
int Api_Print( const char* buf )
{
	return KdPrint( (char*)buf );
}
/* 
//发送消息
int sys_send( void* session, void* content, size_t len, uint flag )
{
	KThread* dest;
	if( !IS_USER_MEMORY((uint)content) || !IS_USER_MEMORY((uint)content+len) )
		return -ERR_WRONGARG;
	dest = (KThread*)((session_t*)session)->thread;
	if( !IS_THREAD(dest) )
		return -ERR_WRONGARG;
	return message_send( session, content, len, flag );
}

//接收消息
int sys_recv( void* session, void* content, size_t* len, uint flag )
{
	KThread* dest;
	KSpace* space;
	space = MmGetCurrentSpace();
	if( !IS_WRITABLE(space, session, sizeof(session_t)) ||
		!IS_WRITABLE(space, content, *len) || 
		!IS_WRITABLE(space, len, sizeof(size_t)) ){
		return -ERR_WRONGARG;
	}
	dest = (KThread*)((session_t*)session)->thread;
	if( dest && !IS_THREAD(dest) )
		return -ERR_WRONGARG;
	return message_recv( session, content, len, flag );
}

//分配用户内存
void* sys_virtual_alloc( size_t siz, size_t addr, uint flag )
{
	void* ptr;
	if( flag & ALLOC_WITH_ADDR ){
		ptr = umalloc_ex( MmGetCurrentSpace(), addr, siz );
	}else{
		ptr = MmAllocateUserMemory( MmGetCurrentSpace(), siz );
	}
	return ptr;
}

//释放内存
void sys_virtual_free( void* p )
{
	MmFreeUserMemory( MmGetCurrentSpace(), p );
}

//退出线程
void sys_thread_exit( int code )
{
	//直接终止
	KThread* thr;
	thr = TmGetCurrentThread();
	if( MmGetCurrentSpace()->main_thread == thr ){
		//terminate the process
		KdPrintf("Program %s exited with code 0x%X\n", MmGetCurrentSpace()->name, code );
	}
	TmTerminateThread( thr, code );
	//process_kill
}

//创建线程
int sys_thread_create( size_t addr, uint* ret )
{
	KThread* thr;
	if( IS_USER_MEMORY(addr) &&
		IS_WRITABLE( MmGetCurrentSpace(), ret, sizeof(uint)) ){
		thr = thread_create( MmGetCurrentSpace(), addr, 0 );
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
	return (uint)TmGetCurrentThread();
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
	KThread* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	thread_suspend( thr );
	return 0;
}

//启动线程
int sys_thread_resume( uint thread )
{
	KThread* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	thread_resume( thr );
	return 0;
}

//结束线程
int sys_thread_kill( uint thread, int code )
{
	KThread* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	TmTerminateThread( thr, code );
	return 0;
}

//设置线程优先级
int sys_thread_set_priority( uint thread, int pri )
{
	KThread* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	if( thr->Space != MmGetCurrentSpace() )
		return -ERR_LOWPRI;
	if( pri<1 || pri>4 )
		return -ERR_WRONGARG;
	if( pri == PRI_REALTIME )
		thr->Space->realtime_thread = thr;
	else{
		if( thr->priority == PRI_REALTIME )
			thr->Space->realtime_thread = NULL;
	}
	thr->priority = pri;
	return 0;
}

//获取线程优先级
int sys_thread_get_priority( uint thread, int* pri )
{
	KThread* thr;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	return thr->priority;
}

//返回一个空闲的信号量
int sys_thread_semget(int value)
{
	KSpace* space;
	int i;
	space = MmGetCurrentSpace();
	down( &space->semaphore );
	for(i=0; i<MAX_SEM_NUM; i++ )
		if( space->sem_array[i]==NULL ){
			space->sem_array[i] = MmAllocateKernelMemory(sizeof(KSemaphore));
			up( &space->semaphore );
			sema_init_ex( space->sem_array[i], value );
			return i;
		}
	up( &space->semaphore );
	return -ERR_NOMEM;
}

int sys_thread_semop( int i, int op )
{
	KSpace* space;
	space = MmGetCurrentSpace();
	if( i<0 || i>=MAX_SEM_NUM || !space->sem_array[i] )
		return -ERR_WRONGARG;
	switch( op ){
	case SEMOP_DOWN:
		IpcLockSemaphore( space->sem_array[i] );
		return 0;
	case SEMOP_UP:
		IpcUnlockSemaphore( space->sem_array[i] );
		return 0;
	case SEMOP_TRYDOWN:
		return sema_trydown( space->sem_array[i] );
	}
	return -ERR_WRONGARG;
}

int sys_thread_semctl( int i, int cmd )
{
	KSpace* space;
	space = MmGetCurrentSpace();
	if( i<0 || i>=MAX_SEM_NUM || !space->sem_array[i] )
		return -ERR_WRONGARG;
	switch( cmd ){
	case SEMCTL_FREE:
		sema_destroy( space->sem_array[i] );
		MmFreeKernelMemory( space->sem_array[i] );
		space->sem_array[i] = NULL;
		break;
	}
	return -ERR_WRONGARG;
}

//创建进程
int sys_process_create( const char* cmdline, const char** var, void* cinfo, uint* retp )
{
	KSpace* curproc, *newproc;
	KThread* init;
	int len;
	env_t* env;
	curproc = MmGetCurrentSpace();
	if( !cmdline || !IS_WRITABLE( curproc, retp, sizeof(uint)) )
		return -ERR_WRONGARG;
	//如果cmdline指向错误路径，则异常退出
	env = (env_t*)MmAllocateKernelMemory( sizeof(env_t) );
	if( !env )
		return -ERR_NOMEM;
	RtlZeroMemory( env, 0, sizeof(env_t) );
	//cmdline
	strncpy( env->cmdline, cmdline, PAGE_SIZE );
	//variables
	if( var ){
		char* p, **q, *end;
		end = env->variables + ENV_VARIABLES_SIZE;
		for( p=env->variables, q=(char**)var; *q && p<end; q++ ){
			len = strlen( *q )+1; //including '\0'
			if( p+len<end ){
				RtlCopyMemory( p, *q, len );
				p+=len;
			}else{
				break;
			}
		}
	}
	//create process
	newproc = MmCreateSpace( curproc );
	if( !newproc ){
		MmFreeKernelMemory( env );
		return -ERR_NOMEM;
	}
	//give the newproc a name
	strncpy( newproc->name, env->cmdline, PROCESS_NAME_LEN-1 );
	//set environment kernel pointer
	newproc->environment = env;
	//space/newproc.c
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
int sys_process_kill( uint space, int code )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//挂起进程
int sys_process_suspend( uint space)
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//启动进程
int sys_process_resume( uint space )
{
	PERROR("not implemented.");
	return -ERR_NOIMP;
}

//当前进程ID
uint sys_process_self()
{
	return (uint)MmGetCurrentSpace();
}

//加载器 返回加载id
int sys_loader_open( char* file, uint* ret_mod )
{
	KSpace* space;
	int ret, len;
	uchar share = 0;
	space = MmGetCurrentSpace();
	if( !IS_WRITABLE(space, ret_mod, sizeof(uint) ) )
		return -ERR_WRONGARG;
	len = strlen(file);
	if( file[len-4]=='.'&&file[len-3]=='b'&&file[len-2]=='x'&&
		file[len-1]=='m' )
		share = 1;
	ret = loader_load( space, file, share, (MODULE**)ret_mod );
	return ret;
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
	KThread* thr;
	int ret;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	if( thr->Space != MmGetCurrentSpace() )
		return -ERR_LOWPRI;
	ret = name_insert( thr, name );
	return ret;
}

int sys_namespace_delete( uint thread, char* name )
{
	KThread* thr;
	int ret;
	if( !IS_THREAD(thread) )
		return -ERR_WRONGARG;
	thr = (KThread*)thread;
	if( thr->Space != MmGetCurrentSpace() )
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
	if( TmGetCurrentThread()->process->UserId != ADMIN_USER )
		return -ERR_LOWPRI;
	if( flag & MAP_UNMAP ){
		ArUnmapMultiplePages( vaddr, paddr, map_size );
	}
	if( flag & MAP_READONLY )
		ArMapMultiplePages( MmGetCurrentSpace()->PageDirectory, vaddr, paddr, map_size, P_USER );
	else
		ArMapMultiplePages( MmGetCurrentSpace()->PageDirectory, vaddr, paddr, map_size, P_USER | P_WRITE );
	return 0;
}

//BIOS调用
int sys_bios_call( int interrupt, void* context, size_t siz )
{
	if( TmGetCurrentThread()->process->UserId != ADMIN_USER )
		return -ERR_LOWPRI;
	if( !IS_WRITABLE( MmGetCurrentSpace(), context, siz ) )
		return -ERR_WRONGARG;
	return bios_call( interrupt, context, siz );
}
*/
