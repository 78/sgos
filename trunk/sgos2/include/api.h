#ifndef _API_H_
#define _API_H_

#include <sgos.h>

#ifndef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif
#endif

#define SYSCALL0(id, type, name) EXTERN type Api_##name()
#define SYSCALL1(id, type, name, atype, a) EXTERN type Api_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) EXTERN type Api_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) EXTERN type Api_##name( atype a, btype b, ctype c )
#define SYSCALL4(id, type, name, atype, a, btype, b, ctype, c, dtype, d) EXTERN type Api_##name( atype a, btype b, ctype c, dtype d )

#include "../kernel/include/apidef.h"

//init.c
ThreadInformation* GetThreadInformation();
ProcessInformation* GetProcessInformation();

//api.c
int SendMessage( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *ret );
int SendMessageEx( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *arg5,
	uint *arg6, uint *arg7, uint* arg8, uint *ret );
void ExitSpace(uint code);
void ExitThread(uint code);
void* AllocateGlobalMemory( size_t siz, uint attr, uint flag );
void FreeGlobalMemory( void* p );
int GetSystemInformation( void* p );
int TerminateThread( uint tid, uint code );
uint GetCurrentThreadId();
int CreateThread( uint sid, size_t proc_addr );
int SleepThread( uint ms );
int WakeupThread( uint tid );
int ResumeThread( uint tid );
int SuspendThread( uint tid );
int JoinThread( uint tid, time_t timeout );
int TerminateSpace( uint sp );
uint GetCurrentSpaceId( );
int CreateSpace( uint parent );
int DestroySpace( uint sp, uint code );
void* AllocateMemory( uint sp, uint siz, uint attr, uint flag );
void FreeMemory( void* ptr );
int WriteMemory( uint sp, size_t addr, void* ptr, size_t count );
int ReadMemory( uint sp, size_t addr, void* ptr, size_t count );
int QueryMemory( uint sp, uint addr, size_t *phys_addr, uint* attr );
int SetMemoryAttribute( uint sp, size_t addr, size_t siz, uint attr );
int AcquirePhysicalPages( uint sp, size_t addr, size_t siz );
int ReleasePhysicalPages( uint sp, size_t addr, size_t siz );
int MapMemory( uint sp, size_t addr, size_t siz, size_t phys_addr, uint attr, uint flag );
int ReplyMessage( Message* msg );
int ReceiveMessage( Message* msg, time_t timeout );

#endif
