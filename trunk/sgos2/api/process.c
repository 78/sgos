#include <sgos.h>
#include <api.h>

uint PsGetCurrentProcessId()
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	if( pi )
		return pi->ProcessId;
	return 0;
}

int PsCreateProcess( const char* cmdline, const char* env, uint * pid )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_Create;
	msg.Arguments[0] = PsGetCurrentProcessId();
	msg.Large[0] = (size_t)SysAllocateMemory( SysGetCurrentSpaceId(), PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
	strcpy( (char*)msg.Large[0], cmdline );
	msg.Large[1] = (size_t)SysAllocateMemory( SysGetCurrentSpaceId(), ENVIRONMENT_STRING_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
	if( env )
		strcpy( (char*)msg.Large[1], env );
	else
		strcpy( (char*)msg.Large[1], "" );
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	SysFreeMemory( SysGetCurrentSpaceId(), (void*)msg.Large[0] );
	SysFreeMemory( SysGetCurrentSpaceId(), (void*)msg.Large[1] );
	if( ret < 0 )
		return ret;
	if( msg.Code<0 )
		return msg.Code;
	if( pid )
		*pid = msg.Code;
	return 0; 
}

int PsTerminateProcess( int pid, int code )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_Terminate;
	msg.Arguments[0] = pid;
	msg.Arguments[1] = code;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}

int PsSuspendProcess( int pid )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_Suspend;
	msg.Arguments[0] = pid;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}

int PsResumeProcess( int pid )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_Resume;
	msg.Arguments[0] = pid;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}

int PsCreateThread( size_t addr )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_CreateThread;
	msg.Arguments[0] = addr;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}

int PsTerminateThread( int tid, int code )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(wProcessId);
	msg.Command = wProcess_TerminateThread;
	msg.Arguments[0] = tid;
	msg.Arguments[1] = code;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}

int PsSuspendThread( int tid )
{
	return SysSuspendThread( tid );
}

int PsResumeThread( int tid )
{
	return SysResumeThread( tid );
}


