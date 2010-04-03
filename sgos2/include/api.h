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
EXTERN ThreadInformation* GetCurrentThreadInformation();
EXTERN ProcessInformation* GetCurrentProcessInformation();

//api.c
EXTERN int SendMessage( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *ret );
EXTERN int SendMessageEx( uint dest, uint cmd, uint *arg1, uint *arg2, uint *arg3, uint *arg4, uint *arg5,
	uint *arg6, uint *arg7, uint* arg8, uint *ret );
EXTERN int ReplyMessage( Message* msg );
EXTERN int ReceiveMessage( Message* msg, time_t timeout );
EXTERN int WaitMessage( Message* msg );

//system api
EXTERN void* SysGetSystemInformation();
EXTERN void SysExitSpace(uint code);
EXTERN void SysExitThread(uint code);
EXTERN void* SysAllocateGlobalMemory( size_t siz, uint attr, uint flag );
EXTERN void SysFreeGlobalMemory( void* p );
EXTERN int SysTerminateThread( uint tid, uint code );
EXTERN uint SysGetCurrentThreadId();
EXTERN int SysCreateThread( uint sid, size_t proc_addr, size_t stack_limit, size_t stack_base, void* ti );
EXTERN int SysSleepThread( uint ms );
EXTERN int SysWakeupThread( uint tid );
EXTERN int SysResumeThread( uint tid );
EXTERN int SysSuspendThread( uint tid );
EXTERN int SysJoinThread( uint tid, time_t timeout );
EXTERN int SysTerminateSpace( uint sp );
EXTERN uint SysGetCurrentSpaceId( );
EXTERN int SysCreateSpace( uint parent );
EXTERN int SysDestroySpace( uint sp, uint code );
EXTERN void* SysAllocateMemory( uint sp, uint siz, uint attr, uint flag );
EXTERN void* SysAllocateMemoryAddress( uint sp, size_t addr, uint siz, uint attr, uint flag );
EXTERN void SysFreeMemory( uint sp, void* ptr );
EXTERN int SysWriteMemory( uint sp, size_t addr, void* ptr, size_t count );
EXTERN int SysReadMemory( uint sp, size_t addr, void* ptr, size_t count );
EXTERN int SysQueryMemory( uint sp, uint addr, size_t *phys_addr, uint* attr );
EXTERN int SysSetMemoryAttribute( uint sp, size_t addr, size_t siz, uint attr );
EXTERN int SysAcquirePhysicalPages( uint sp, size_t addr, size_t siz );
EXTERN int SysReleasePhysicalPages( uint sp, size_t addr, size_t siz );
EXTERN int SysMapMemory( uint sp, size_t addr, size_t siz, size_t phys_addr, uint attr, uint flag );
EXTERN int SysSwapMemory( uint dest_sp, size_t dest_addr, size_t src_addr, size_t siz, uint flag );
EXTERN int SysDuplicateMemory( uint dest_sp, size_t dest_addr, size_t src_addr, size_t siz );
EXTERN int SysQueryAddress( uint dest_sp, size_t dest_addr, size_t *rbeg, size_t*rend, uint *attr, uint* flag );

//clock.c
EXTERN uint _GetTickCount();

//service.c
EXTERN uint SmGetServiceThreadById( uint serviceId );
EXTERN uint SmGetServiceThreadByName( const char* name );
EXTERN int SmRemoveService( uint serviceId );
EXTERN int SmNotifyService( uint serviceId, uint eventFlag, const char* name );

//filesystem.c
EXTERN FILEBUF* FsOpenFile( const char* fname, uint mode, uint flag );
EXTERN void FsCloseFile( FILEBUF* fb );
EXTERN int FsReadFile( FILEBUF* fb, uchar* buf, int count );
EXTERN int FsWriteFile( FILEBUF* fb, uchar* buf, int count );
EXTERN int FsSetFilePointer( FILEBUF* fb, int pos, int method );
EXTERN int FsSetFileSize( FILEBUF* fb, size_t newsize );
EXTERN int FsControlFile( FILEBUF* fb, uint cmd, uint arg );
EXTERN int FsReadDirectory( FILEBUF* fb, DIRENTRY* buf, int count );

//process.c
EXTERN uint PsGetCurrentProcessId();
EXTERN int PsCreateProcess( const char* cmdline, const char* env, uint * pid );
EXTERN int PsTerminateProcess( int pid, int code );
EXTERN int PsSuspendProcess( int pid );
EXTERN int PsResumeProcess( int pid );
EXTERN int PsCreateThread( size_t addr );
EXTERN int PsTerminateThread( int tid, int code );
EXTERN int PsSuspendThread( int tid );
EXTERN int PsResumeThread( int tid );
EXTERN int PsLoadModule( const char* path );
EXTERN void PsFreeModule( int mid );
EXTERN int PsGetModule( const char* name, char* path, int path_len );
EXTERN size_t PsGetProcedure( int mid, const char* name );

#endif
