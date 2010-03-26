#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>
#include "module.h"
#include "wprocess.h"

static Process* firstProcess = 0;

Process* Process::EnumProcessTree( Process* p, uint id )
{
	Process* ret;
	for( Process * t=p; t; t=t->next ){
		if( t->spaceId == id )
			return t;
		if( t->child ){
			 ret = EnumProcessTree( t->child, id );
			 if( ret )
				return ret;
		}
	}
	return 0;
}

Process* Process::GetProcessById( uint id )
{
	return EnumProcessTree( firstProcess, id );
}

void Process::KillProcessChildren( Process* p )
{
	for(Process *t=p->child; t; t=t->next ){
		KillProcessChildren( t );
		t->child = 0;
		t->Dispose();
	}
}

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
	ti->ProcessInformation = ps->GetInformation()->Self;
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
	this->process = ps;
	if( ps->mainThread ){
		Thread* t;
		for( t=ps->mainThread; t->next; t=t->next );
		t->next = this;
		this->next = 0;
		this->prev = t;
	}else{
		this->prev = this->next = 0;
		ps->mainThread = this;
	}
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
	if( this->prev )
		this->prev->next = this->next;
	else
		process->mainThread = this->next;
	if( this->next )
		this->next->prev = this->prev;
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

Process::Process( uint pid, char* cmdline, char* env )
{
	this->disposed = false;
	this->spaceId = 0;
	this->module = 0;
	this->pi = 0;
	this->commandLine = this->environment = 0;
	this->prev = this->next = this->parent = this->child = 0;
	if( Initialize( pid, cmdline, env ) < 0 )
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
	//remove from tree
	if( this==firstProcess ){
		firstProcess = this->next;
	}else if( this->parent || this->prev ){
		if( this->prev )
			this->prev->next = this->next;
		if( this->next )
			this->next->prev = this->prev;
	}
	KillProcessChildren( this );
}

static void CopyCommandName( char* to, char* src )
{
	char* i = strchr( src, ' ' );
	if( i ){
		*i=0;
		strncpy( to, src, PATH_LEN-1 );
		*i= ' ';
	}else{
		strncpy( to, src, PATH_LEN-1 );
	}
}

// How to initialize a process ?
int Process::Initialize( uint pid, char* cmdline, char* env )
{
	Process* p = (Process*)0;
	uint tid;
	int result;
	size_t entry, remote_pi;
	if( this->disposed )
		return -ERR_DISPOSED;
	//1. create a space for the process
	uint sid = SysCreateSpace( 0 );
	if( sid < 0 )
		goto bed;
	this->spaceId = sid;
	//3. load executable file to the space!
	CopyCommandName( this->modulePath, cmdline );
	if( (result = PeLoadLibrary( sid, this->modulePath ) ) < 0 )
		goto bed;
	this->module = GetModuleById( result );
	if( !this->module ){
		printf("## get module by id failed.\n");
		goto bed;
	}
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
	pi->Self = (ProcessInformation*)remote_pi;
	if( cmdline && (result=MapAddress(SysGetCurrentSpaceId(), sid, (size_t)cmdline, (size_t*)&pi->CommandLine )) < 0 )
		goto bed;
	if( env && (result=MapAddress(SysGetCurrentSpaceId(), sid, (size_t)env, (size_t*)&pi->EnvironmentVariables )) < 0 )
		goto bed;
	//5. create a main thread
	printf("[wprocess]sp: %x entry: %x\n", sid, this->module->EntryAddress ); 
	CreateThread( this->module->EntryAddress );
	if( this->mainThread==0 )
		goto bed;
	pi->MainThreadId = this->mainThread->ThreadId();
	//7. save informations
	this->commandLine = cmdline;
	this->environment = env;
	//8. start main thread to start the process
	this->mainThread->Resume();
	//9. add to tree
	if( pid )
		p = GetProcessById( pid );
	if( !p )
		p = firstProcess;
	if( !p ){
		firstProcess = this;
		this->prev = this->next = this->child = this->parent = 0;
	}else{
		while( p->next )
			p = p->next;
		p->next = this;
		this->prev = p;
		this->child = 0;
		this->parent = p->parent;
	}
	return 0;
bed:
	printf("[wprocess] failed. result=%d\n", result );
	Dispose();
	return -ERR_UNKNOWN;
}

Thread* Process::CreateThread( size_t entry )
{
	Thread* t = new Thread( this, entry );
	return t;
}

Thread* Process::GetThreadById( uint tid )
{
	for( Thread* t=this->mainThread; t; t=t->NextThread() )
		if( t->ThreadId() == tid )
			return t;
	return 0;
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

