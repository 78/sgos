//API for SGOS2

#include "apiImplement.h"
#include "../kernel/include/apidef.h"
#include <stdio.h>

int ReplyMessage( Message* msg )
{
	return Api_Send( msg, 0 );
}

int WaitMessage( Message* msg )
{
	msg->ThreadId = ANY_THREAD;
	msg->Command = 0;
	return ReceiveMessage( msg, INFINITE );
}

int ReceiveMessage( Message* msg, time_t timeout )
{
	return Api_Receive( msg, timeout );
}

int SendMessage( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *ret )
{
	int result;
	Message msg = { SystemId, 0};
	msg.ThreadId = dest;
	msg.Command = cmd;
	if( arg1 )
		msg.Arguments[0] = *arg1;
	if( arg2 )
		msg.Arguments[1] = *arg2;
	if( arg3 )
		msg.Arguments[2] = *arg3;
	if( arg4 )
		msg.Arguments[3] = *arg4;
	result = Api_Send( &msg, 0 );
	if( result < 0 ){
		printf("[api]Failed to Send Message %x result=%d\n", cmd, result );
		return result;
	}
	result = Api_Receive( &msg, 3*1000 ); //3 seconds.
	if( result == ERR_TIMEOUT ){
		printf("[api]Receive timeout. cmd:%x\n", cmd );
	}
	if( arg1 )
		*arg1 = msg.Arguments[0];
	if( arg2 )
		*arg2 = msg.Arguments[1];
	if( arg3 )
		*arg3 = msg.Arguments[2];
	if( arg4 )
		*arg4= msg.Arguments[3];
	if( ret )
		*ret = msg.Code;
	return result;
}

int SendMessageEx( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *arg5,
	uint *arg6, uint *arg7, uint* arg8, uint *ret )
{
	int result;
	Message msg = { SystemId, 0};
	msg.ThreadId = dest;
	msg.Command = cmd;
	if( arg1 )
		msg.Arguments[0] = *arg1;
	if( arg2 )
		msg.Arguments[1] = *arg2;
	if( arg3 )
		msg.Arguments[2] = *arg3;
	if( arg4 )
		msg.Arguments[3] = *arg4;
	if( arg5 )
		msg.Arguments[4] = *arg5;
	if( arg6 )
		msg.Arguments[5] = *arg6;
	if( arg7 )
		msg.Arguments[6] = *arg7;
	if( arg8 )
		msg.Arguments[7] = *arg8;
	result = Api_Send( &msg, 0 );
	if( result < 0 ){
		printf("[servicemanager]Failed to Send Message %x result=%d\n", cmd, result );
		return result;
	}
	result = Api_Receive( &msg, 3*1000 ); //3 seconds.
	if( result == ERR_TIMEOUT ){
		printf("[servicemanager]Receive timeout. cmd:%d\n", cmd );
	}
	if( arg1 )
		*arg1 = msg.Arguments[0];
	if( arg2 )
		*arg2 = msg.Arguments[1];
	if( arg3 )
		*arg3 = msg.Arguments[2];
	if( arg4 )
		*arg4= msg.Arguments[3];
	if( arg5 )
		*arg5= msg.Arguments[4];
	if( arg6 )
		*arg6= msg.Arguments[5];
	if( arg7 )
		*arg7= msg.Arguments[6];
	if( arg8 )
		*arg8= msg.Arguments[7];
	if( ret )
		*ret = msg.Code;
	return result;
}

void* SysGetSystemInformation()
{
	void* ptr=NULL;
	ProcessInformation* pi = (ProcessInformation*)GetCurrentProcessInformation();
	if( pi ){
		return pi->SystemInformation;
	}
	if( SendMessage( SystemId, System_GetSystemInformation, NULL, NULL, NULL, NULL, (void*)&ptr ) < 0 )
		return NULL;
	return ptr;
}

void SysEnableInterrupt(int no, int b)
{
	if( SendMessage( SystemId, System_EnableInterrupt, &no, &b, NULL, NULL, NULL ) < 0 )
		printf("[api]failed to send message.\n");
}

int SysAddInterruptHandler(int no, uint tid)
{
	int code;
	uint result = SendMessage( SystemId, System_AddInterruptHandler, (void*)&no, &tid, 
		NULL, NULL, (void*)&code );
	if( result < 0 )
		return result;
	return code;
}

void SysDelInterruptHandler(int no, uint tid)
{
	SendMessage( SystemId, System_DelInterruptHandler, &no, &tid, NULL, NULL, NULL );
}


void SysExitSpace(uint code)
{
	SendMessage( SystemId, System_ExitSpace, &code, NULL, NULL, NULL, NULL );
}

void SysExitThread(uint code)
{
	SendMessage( SystemId, System_ExitThread, &code, NULL, NULL, NULL, NULL );
}

void* SysAllocateGlobalMemory( size_t siz, uint attr, uint flag )
{
	void* ptr;
	uint result = SendMessage( SystemId, System_AllocateGlobalMemory, (void*)&siz, &attr, 
		&flag, NULL, (void*)&ptr );
	if( result < 0 )
		return NULL;
	return ptr;
}

void SysFreeGlobalMemory( void* p )
{
	SendMessage( SystemId, System_FreeGlobalMemory, (void*)&p, NULL, NULL, NULL, NULL );
}


int SysTerminateThread( uint tid, uint code )
{
	int result;
	result = SendMessage( SystemId, System_TerminateThread, &tid, &code, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

uint SysGetCurrentThreadId()
{
	int result;
	uint code;
	ThreadInformation* ti = (ThreadInformation*)GetCurrentThreadInformation();
	if( ti ){
		return ti->ThreadId;
	}
	result = SendMessage( SystemId, System_GetCurrentThreadId, NULL, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysCreateThread( uint sid, size_t proc_addr, size_t stack_limit, size_t stack_base, void* ti )
{
	int result;
	uint code;
	result = SendMessageEx( SystemId, System_CreateThread, &sid, (void*)&proc_addr, (void*)&stack_limit, 
		(void*)&stack_base, (void*)&ti, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysSleepThread( uint ms )
{
	Message msg = {SystemId, 0, System_SleepThread};
	msg.Arguments[0] = ms;
	int result = Api_Send( &msg, INFINITE );
	return result;
}

int SysWakeupThread( uint tid )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_WakeupThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysResumeThread( uint tid )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_ResumeThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysSuspendThread( uint tid )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_SuspendThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysJoinThread( uint tid, time_t timeout )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_JoinThread, (void*)&tid, (void*)&timeout, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysTerminateSpace( uint sp )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_TerminateSpace, &sp, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

uint SysGetCurrentSpaceId( )
{
	int result;
	uint code;
	static uint sid = 0;
	ThreadInformation* ti = (ThreadInformation*)GetCurrentThreadInformation();
	if( ti ){
		return ti->SpaceId;
	}
	if( sid )
		return sid;
	result = SendMessage( SystemId, System_GetCurrentSpaceId, NULL, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	sid = code;
	return code;
}

int SysCreateSpace( uint parent )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_CreateSpace, &parent, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysDestroySpace( uint sp, uint code )
{
	int result;
	result = SendMessage( SystemId, System_DestroySpace, &sp, &code, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

void* SysAllocateMemory( uint sp, uint siz, uint attr, uint flag )
{
	int result;
	void* ptr;
	result = SendMessage( SystemId, System_AllocateMemory, &sp, &siz, &attr, &flag, (void*)&ptr );
	if( result < 0 )
		return NULL;
	return ptr;
}

void* SysAllocateMemoryAddress( uint sp, size_t addr, uint siz, uint attr, uint flag )
{
	int result;
	void* ptr;
	result = SendMessageEx( SystemId, System_AllocateAddress, (void*)&sp, (void*)&addr, (void*)&siz, (void*)&attr, (void*)&flag, 
		NULL, NULL, NULL, (void*)&ptr );
	if( result < 0 )
		return NULL;
	return ptr;
}

void SysFreeMemory( uint sp, void* ptr )
{
	SendMessage( SystemId, System_FreeMemory, &sp, (void*)&ptr, NULL, NULL, NULL );
}

int SysQueryMemory( uint sp, uint addr, size_t *phys_addr, uint* attr )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_QueryMemory, &sp, &addr, (void*)phys_addr, (void*)attr, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysAcquirePhysicalPages( uint sp, size_t addr, size_t siz )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_AcquirePhysicalPages, (void*)&siz, (void*)&addr, (void*)&siz, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysReleasePhysicalPages( uint sp, size_t addr, size_t siz )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_ReleasePhysicalPages, (void*)&siz, (void*)&addr, (void*)&siz, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysMapMemory( uint sp, size_t addr, size_t siz, size_t phys_addr, uint attr, uint flag )
{
	int result;
	int code;
	result = SendMessageEx( SystemId, System_MapMemory, (void*)&sp, (void*)&addr, (void*)&siz, (void*)&phys_addr, (void*)&attr, 
		(void*)&flag, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SysSwapMemory( uint dest_sp, size_t dest_addr, size_t src_addr, size_t siz, uint flag )
{
	int result;
	int code;
	result = SendMessageEx( SystemId, System_SwapMemory, (void*)&dest_sp, (void*)&dest_addr, (void*)&src_addr, 
		(void*)&siz, (void*)&flag, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}


int SysDuplicateMemory( uint dest_sp, size_t dest_addr, size_t src_addr, size_t siz )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_DuplicateMemory, (void*)&dest_sp, (void*)&dest_addr, (void*)&src_addr, 
		(void*)&siz, (void*)&code );
	if( result < 0 )
		return result;
	return code;
}


int SysQueryAddress( uint dest_sp, size_t dest_addr, size_t *rbeg, size_t*rend, uint *attr, uint* flag )
{
	int result;
	int code;
	result = SendMessageEx( SystemId, System_QueryAddress, (void*)&dest_sp, (void*)&dest_addr, (void*)rbeg, 
		(void*)rend, (void*)attr, (void*)flag, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}
