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
	Api_Send,
	Api_Receive,
	Api_InvokeBiosService,
};

//返回计数
uint Api_Test()
{
	static unsigned counter=0; 
	return counter++;
}

//输出调试信息
int Api_Print( const char* buf, int nbytes )
{
	int i;
	for( i=0; i<nbytes && buf[i]; i++ )
		 KdPrintf("%c", buf[i] );
	return nbytes;
}

//Send a message
int Api_Send( Message* msg, time_t timeout )
{
	if( (size_t)msg + sizeof(Message) >= KERNEL_BASE )
		return -ERR_WRONGARG;
	return IpcCall( msg, 0, timeout);
}

//Receive a message
int Api_Receive( Message* msg, time_t timeout )
{
	if( (size_t)msg + sizeof(Message) >= KERNEL_BASE )
		return -ERR_WRONGARG;
	return IpcReceive( msg, 0, timeout );
}

int Api_InvokeBiosService( int int_no, void* context, size_t context_size )
{
	KVirtualMemoryAllocation* vma;
	vma = MmGetVmaByAddress( &MmGetCurrentSpace()->VirtualMemory, (size_t)context );
	if( !vma || vma->VirtualAddress+vma->VirtualSize < (size_t)context + context_size )
		return -ERR_WRONGARG;
	return ArInvokeBiosService( int_no, context, context_size );
}

