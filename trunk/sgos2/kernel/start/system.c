#include <sgos.h>
#include <ipc.h>
#include <tm.h>
#include <mm.h>
#include <kd.h>

//Static variables
static KThread* SystemThread;

static int Reply(Message* msg, int code)
{
	msg->Code = (uint)code;
	return IpcSend( msg, 0 );
}

//Thread Management Message
static void DoTmMessage(Message* msg )
{
	int result = 0;
	KThread* thread;
	switch( msg->Command ){
	case System_ExitThread:
		result = TmTerminateThread( TmGetThreadById(msg->ThreadId),
			msg->Arguments[0] );
		return;
	case System_TerminateThread:
		result = TmTerminateThread( TmGetThreadById(msg->Arguments[0]),
			msg->Arguments[1] );
		break;
	case System_GetCurrentThreadId:
		result = msg->ThreadId;
		break;
	case System_CreateThread:
		thread = TmCreateAdvancedThread( MmGetSpaceById(msg->Arguments[0]), msg->Arguments[1], msg->Arguments[2], 
			msg->Arguments[3], (ThreadInformation*)msg->Arguments[4], USER_THREAD );
		if( thread == NULL )
			result = -ERR_UNKNOWN;
		else
			result = thread->ThreadId;
		break;
	case System_SleepThread:
		//Note: SleepThread  no reply because a message reply will wakup the thread.
		thread = TmGetThreadById( msg->ThreadId );
		result = TmSleepThread( thread, msg->Arguments[0] );
		return; 
	case System_WakeupThread:
		thread = TmGetThreadById( msg->Arguments[0] );
		if( thread == NULL )
			result = -ERR_WRONGARG;
		else
			result = TmWakeupThread( thread );
		break;
	case System_ResumeThread:
		thread = TmGetThreadById( msg->Arguments[0] );
		if( thread == NULL )
			result = -ERR_WRONGARG;
		else
			result = TmResumeThread( thread );
		break;
	case System_SuspendThread:
		thread = TmGetThreadById( msg->Arguments[0] );
		if( thread == NULL )
			result = -ERR_WRONGARG;
		else
			result = TmSuspendThread( thread );
		break;
	case System_JoinThread:
		thread = TmGetThreadById( msg->Arguments[0] );
		if( thread == NULL )
			result = -ERR_WRONGARG;
		else
			TmJoinThread( thread );
		break;
	default:
		PERROR("## What? command = 0x%x", msg->Command );
	}
	Reply( msg, result );
}

//Memory Management Message
static void DoMmMessage(Message* msg )
{
	int result = 0;
	KSpace* space = MmGetSpaceById( msg->ThreadId>>16 );
	if( space == NULL ){
		PERROR("## space == NULL");
		return;
	}
	switch( msg->Command ){
	case System_ExitSpace:
		PERROR("Exit Space");
		TmTerminateThread( TmGetThreadById(msg->ThreadId),
			msg->Arguments[0] );
		MmDestroySpace( space );
		return;
	case System_DestroySpace:
		MmDestroySpace( MmGetSpaceById(msg->Arguments[0]) );
		break;
	case System_AllocateGlobalMemory:
		result = (uint)MmAllocateGlobalMemory( msg->Arguments[0], msg->Arguments[1], msg->Arguments[2] );
		break;
	case System_FreeGlobalMemory:
		MmFreeGlobalMemory( (void*)msg->Arguments[0] );
		break;
	case System_GetCurrentSpaceId:
		result = space->SpaceId;
		break;
	case System_CreateSpace:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else{
			space = MmCreateSpace( space );
			if( space )
				result = space->SpaceId;
			else
				result = -ERR_UNKNOWN;
		}
		break;
	case System_AllocateMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = (uint)MmAllocateUserMemory( space, msg->Arguments[1], 
				msg->Arguments[2], msg->Arguments[3] );
		break;
	case System_AllocateAddress:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = (uint)MmAllocateUserMemoryAddress( space, msg->Arguments[1], 
				msg->Arguments[2], msg->Arguments[3], msg->Arguments[4] );
		break;
	case System_FreeMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			MmFreeUserMemory( space, (void*)msg->Arguments[1] );
		break;
	case System_QueryMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = ArQueryPageInformation( &space->PageDirectory, msg->Arguments[1],
				&msg->Arguments[2], &msg->Arguments[3] );
		break;
	case System_AcquirePhysicalPages:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = MmAcquireMultiplePhysicalPages( space, msg->Arguments[1], 
				msg->Arguments[2], 0, 0 );
		break;
	case System_ReleasePhysicalPages:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			MmReleaseMultiplePhysicalPages( space, msg->Arguments[1], 
				msg->Arguments[2] );
		break;
	case System_MapMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = MmMapMemory( space, msg->Arguments[1], msg->Arguments[3],
				msg->Arguments[2], msg->Arguments[4], msg->Arguments[5] );
		break;
	case System_DuplicateMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = MmDuplicateMultiplePhysicalPages( space, msg->Arguments[1], MmGetSpaceById(SPACEID(msg->ThreadId)),
				msg->Arguments[2], msg->Arguments[3] );
		break;
	case System_SwapMemory:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = MmSwapMultiplePhysicalPages( space, msg->Arguments[1], MmGetSpaceById(SPACEID(msg->ThreadId)),
				msg->Arguments[2], msg->Arguments[3], msg->Arguments[4] );
		break;
	case System_QueryAddress:
		space = MmGetSpaceById( msg->Arguments[0] );
		if( space == NULL )
			result = -ERR_WRONGARG;
		else
			result = MmQueryVirtualAddressInformation( &space->VirtualMemory, msg->Arguments[1], &msg->Arguments[2],
				&msg->Arguments[3], &msg->Arguments[4], &msg->Arguments[5] );
		break;
	default:
		PERROR("## What? command = 0x%x", msg->Command );
	}
	Reply( msg, result );
}

//System Information Message
static void DoSystemInformationMessage(Message* msg )
{
	switch( msg->Command ){
	case System_GetSystemInformation:
		;
	}
}

//System Message Handler
static void SystemMessageLoop()
{
	Message msg;
	KdPrintf("System thread started.\n");
	for(;;){
		msg.ThreadId = ANY_THREAD;
		int result = IpcReceive( &msg, 0, INFINITE );
		uint cmd = msg.Command;
		if( result<0 ){
			PERROR("Failed to receive message. result=0x%X", result );
			continue;
		}
//		KdPrintf("System Cmd: 0x%X\n", cmd );
		if( cmd < 0x1000 ){ //System Information
			DoSystemInformationMessage(&msg);
		}else if( cmd < 0x2000 ){
			DoTmMessage(&msg);
		}else if( cmd < 0x3000 ){
			DoMmMessage(&msg);
		}
	}
}

//Start a system thread for handling system message
int KeStartSystemThread()
{
	SystemThread = TmCreateThread( MmGetCurrentSpace(), (size_t)&SystemMessageLoop, KERNEL_THREAD );
	SystemThread->ThreadId = 0;
	TmResumeThread( SystemThread );
	return 0;
}


