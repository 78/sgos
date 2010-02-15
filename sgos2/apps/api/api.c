//API for SGOS2

#include "apiImplement.h"
#include "../../kernel/include/apidef.h"

int ReplyMessage( Message* msg )
{
	return Api_Send( msg, 0 );
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
		printf("[servicemanager]Failed to Send Message %x result=%d\n", cmd, result );
		return result;
	}
	result = Api_Receive( &msg, 3*1000 ); //3 seconds.
	if( result == ERR_TIMEOUT ){
		printf("[servicemanager]Receive timeout. cmd:%x\n", cmd );
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

void ExitSpace(uint code)
{
	SendMessage( SystemId, System_ExitSpace, &code, NULL, NULL, NULL, NULL );
}

void ExitThread(uint code)
{
	SendMessage( SystemId, System_ExitThread, &code, NULL, NULL, NULL, NULL );
}

void* AllocateGlobalMemory( size_t siz, uint attr, uint flag )
{
	void* ptr;
	uint result = SendMessage( SystemId, System_AllocateGlobalMemory, &siz, &attr, 
		&flag, NULL, (void*)&ptr );
	if( result < 0 )
		return NULL;
	return ptr;
}

void FreeGlobalMemory( void* p )
{
	SendMessage( SystemId, System_FreeGlobalMemory, (void*)&p, NULL, NULL, NULL, NULL );
}

int GetSystemInformation( void* p )
{
	//System_GetSystemInformation
}

int TerminateThread( uint tid, uint code )
{
	int result;
	result = SendMessage( SystemId, System_TerminateThread, &tid, &code, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

uint GetCurrentThreadId()
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_GetCurrentThreadId, NULL, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int CreateThread( uint sid, size_t proc_addr )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_CreateThread, &sid, &proc_addr, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SleepThread( uint ms )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_SleepThread, &ms, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int WakeupThread( uint tid )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_WakeupThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int ResumeThread( uint tid )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_ResumeThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int SuspendThread( uint tid )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_SuspendThread, &tid, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int JoinThread( uint tid, time_t timeout )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_JoinThread, &tid, &timeout, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int TerminateSpace( uint sp )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_TerminateSpace, &sp, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

uint GetCurrentSpaceId( )
{
	int result;
	uint code;
	result = SendMessage( SystemId, System_GetCurrentSpaceId, NULL, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int CreateSpace( uint parent )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_CreateSpace, &parent, NULL, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int DestroySpace( uint sp, uint code )
{
	int result;
	result = SendMessage( SystemId, System_DestroySpace, &sp, &code, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

void* AllocateMemory( uint sp, uint siz, uint attr, uint flag )
{
	int result;
	void* ptr;
	result = SendMessage( SystemId, System_AllocateMemory, &sp, &siz, &attr, &flag, (void*)&ptr );
	if( result < 0 )
		return NULL;
	return ptr;
}

void FreeMemory( uint sp, void* ptr )
{
	SendMessage( SystemId, System_FreeMemory, &sp, (void*)&ptr, NULL, NULL, NULL );
}

int WriteMemory( uint sp, size_t addr, void* ptr, size_t count )
{
	int code;
	SendMessage( SystemId, System_WriteMemory, &sp, &addr, (void*)&ptr, &count, &code );
	return code;
}

int ReadMemory( uint sp, size_t addr, void* ptr, size_t count )
{
	int code;
	SendMessage( SystemId, System_ReadMemory, &sp, &addr, (void*)&ptr, &count, &code );
	return code;
}

int QueryMemory( uint sp, uint addr, size_t *phys_addr, uint* attr )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_QueryMemory, &sp, &addr, (void*)phys_addr, (void*)attr, &code );
	if( result < 0 )
		return result;
	return code;
}

int SetMemoryAttribute( uint sp, size_t addr, size_t siz, uint attr )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_SetMemoryAttribute, &siz, &addr, &siz, &attr, &code );
	if( result < 0 )
		return result;
	return code;
}

int AcquirePhysicalPages( uint sp, size_t addr, size_t siz )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_AcquirePhysicalPages, &siz, &addr, &siz, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int ReleasePhysicalPages( uint sp, size_t addr, size_t siz )
{
	int result;
	int code;
	result = SendMessage( SystemId, System_ReleasePhysicalPages, &siz, &addr, &siz, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

int MapMemory( uint sp, size_t addr, size_t siz, size_t phys_addr, uint attr, uint flag )
{
	int result;
	int code;
	result = SendMessageEx( SystemId, System_MapMemory, &sp, &addr, &siz, &phys_addr, &attr, 
		&flag, NULL, NULL, &code );
	if( result < 0 )
		return result;
	return code;
}

