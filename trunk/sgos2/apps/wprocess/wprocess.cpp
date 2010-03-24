#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>
#include "module.h"
#include "wprocess.h"

static int MapAddress( uint fromSpace, uint toSpace, size_t remote_addr, size_t * ret_addr )
{
	size_t addr;
	uint phys_addr, attr;
	addr = (size_t)SysAllocateMemory( toSpace, PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_VIRTUAL );
	if( addr == 0 )
		return -1;
	if( SysQueryMemory( fromSpace, remote_addr, &phys_addr, &attr ) <0 )
		return -2;
	if( SysMapMemory( toSpace, addr, PAGE_SIZE, phys_addr, 0, MAP_ADDRESS ) <0 )
		return -3;
	*ret_addr = addr;
	return 0;
}

Thread::Thread( Process * ps, size_t entry )
{
	int sid = ps->SpaceId(), tid;
	this->disposed = false;
	ti = 0;
	//setup thread information block Tib
	size_t remote_addr = (size_t)SysAllocateMemory( sid, PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_HIGHMEM );
	if( remote_addr ==0  )
		goto bed;
	if( MapAddress( sid, SysGetCurrentSpaceId(), remote_addr, (size_t*)&ti ) < 0 )
		goto bed;
	memset( (void*)ti, 0, sizeof(ThreadInformation) );
	ti->StackLimit = MB(2);
	ti->StackBase = (size_t)SysAllocateMemory( sid, ti->StackLimit, MEMORY_ATTR_WRITE, ALLOC_LAZY );
	ti->ProcessInformation = ps->GetInformation();
	ti->Self = (ThreadInformation*)remote_addr;
	ti->Environment = 0;
	ti->ProcessId = ps->ProcessId();
	ti->ErrorCode = 0;
	ti->EntryAddress = entry;
	ti->SpaceInformation = 0;
	ti->SpaceId = sid;
	tid = SysCreateThread( sid, entry, ti->StackLimit, ti->StackBase, (void*)remote_addr );
	this->threadId = tid;
	this->spaceId = sid;
	ti->ThreadId = tid;
	if( tid < 0 )
		goto bed;
	return;
bed:
	Dispose();
}

Thread::~Thread(){
	Dispose();
}

void Thread::Dispose()
{
	int tid = this->threadId, sid=this->spaceId;
	if( this->disposed )
		return;
	this->disposed = true;
	if( tid > 0 )
		SysTerminateThread( sid, tid );
	if( ti && ti->StackBase )
		SysFreeMemory( sid, (void*)ti->StackBase );
	if( ti->Self )
		SysFreeMemory( sid, (void*)ti->Self );
	if( ti )
		SysFreeMemory( SysGetCurrentSpaceId(), this->ti );
	tid = 0;
}

int Thread::Terminate( int code )
{
	this->exitCode = code;
	Dispose();
}

void Thread::Suspend()
{
	if( this->threadId > 0 )
		SysSuspendThread( this->threadId );
}

void Thread::Resume()
{
	if( this->threadId > 0 )
		SysResumeThread( this->threadId );
}

Process::Process( char* cmdline, char* env )
{
	this->disposed = false;
	this->spaceId = 0;
	this->module = 0;
	this->pi = 0;
	this->commandLine = this->environment = 0;
	if( Initialize( cmdline, env ) < 0 )
		Dispose();
}
	
Process::~Process()
{
	Dispose();
}

void Process::Dispose()
{
	if( this->disposed )
		return;
	this->disposed = true;
	for( Thread* t = this->mainThread; t; ){
		Thread* d = t;
		t = t->NextThread();
		delete d;
	}
	if( this->module ){
		FreeModule( this->module, this->spaceId );
	}
	if( this->spaceId ){
		SysDestroySpace( this->spaceId, this->exitCode );
		this->spaceId = 0;
	}
	if( this->pi ){
		SysFreeMemory( SysGetCurrentSpaceId(), this->pi );
	}
	//release resources
	if( this->commandLine )
		SysFreeMemory( SysGetCurrentSpaceId(), this->commandLine );
	if( this->environment )
		SysFreeMemory( SysGetCurrentSpaceId(), this->environment );
	this->commandLine = 0;
	this->environment = 0;
}
	
// How to initialize a process ?
int Process::Initialize( char* cmdline, char* env )
{
	int tid, result;
	size_t entry, remote_pi;
	if( this->disposed )
		return -ERR_DISPOSED;
	//1. create a space for the process
	int sid = SysCreateSpace( 0 );
	if( sid < 0 )
		goto bed;
	this->spaceId = sid;
	//3. load executable file to the space!
	if( (result = PeLoadLibrary( sid, cmdline ) ) < 0 )
		goto bed;
	this->module = GetModuleById( result );
	//4. setup process information block  Pib
	remote_pi = (size_t)SysAllocateMemory( sid, PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_HIGHMEM );
	if( remote_pi == 0 )
		goto bed;
	if( (result=MapAddress( sid, SysGetCurrentSpaceId(), remote_pi, (size_t*)&this->pi )) < 0 )
		goto bed;
	memset( (void*)pi, 0, sizeof(ProcessInformation) );
	pi->ProcessId = sid;
	pi->UserId = 
	pi->GroupId = 0;
	pi->ModuleId = this->module->ModuleId;
	pi->ParentProcessId = 0;
	pi->EntryAddress = this->module->EntryAddress;
	if( cmdline && (result=MapAddress(SysGetCurrentSpaceId(), sid, (size_t)cmdline, (size_t*)&pi->CommandLine )) < 0 )
		goto bed;
	if( env && (result=MapAddress(SysGetCurrentSpaceId(), sid, (size_t)env, (size_t*)&pi->EnvironmentVariables )) < 0 )
		goto bed;
	//5. create a main thread
	printf("[wprocess]sp: %d entry: %x\n", sid, this->module->EntryAddress ); 
	this->mainThread = new Thread( this, this->module->EntryAddress );
	if( this->mainThread==0 )
		goto bed;
	pi->MainThreadId = this->mainThread->ThreadId();
	//7. save informations
	this->commandLine = cmdline;
	this->environment = env;
	//8. start main thread to start the process
	this->mainThread->Resume();
	return 0;
bed:
	printf("[wprocess] failed. result=%d\n", result );
	Dispose();
	return -ERR_UNKNOWN;
}

int Process::CreateThread( size_t entry )
{
	int tid;
	Thread* t = new Thread( this, entry );
	if( t == 0 ){
		return -ERR_NOMEM;
	}
	return t->ThreadId();
}

void Process::Terminate( int code )
{
	this->exitCode = code;
	Dispose();
}

void Process::Suspend()
{
	for( Thread* t=this->mainThread; t; t=t->NextThread() )
		t->Suspend();
}

void Process::Resume()
{
	for( Thread* t=this->mainThread; t; t=t->NextThread() )
		t->Resume();
}

void ProcessTest()
{
	for(;;){
		char* cmdline = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), PAGE_SIZE, MEMORY_ATTR_WRITE, 0 );
		strcpy( cmdline, "/c:/sgos/hello.exe" );
		Process *p=0;
		p = new Process( cmdline, 0 );
		if( p->ProcessId()>0 )
			break;
		delete p;
		SysSleepThread(200);
		printf("destroyed.\n");
	}
}

