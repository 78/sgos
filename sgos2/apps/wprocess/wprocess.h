#ifndef _WPROCESS_H
#define _WPROCESS_H

#include <sgos.h>
#include "module.h"

class Process;
class Thread{
private:
	uint threadId;
	uint spaceId;
	ThreadInformation* ti;
	uint exitCode;
	Thread* next, *prev;
	bool disposed;
	Process* process;
public:
	Thread( Process * ps, size_t entry );
	~Thread();
	void Dispose();
	int Terminate( int code );
	void Resume();
	void Suspend();
	uint ThreadId(){
		return threadId;
	}
	Thread* NextThread(){
		return next;
	}
};

class Process{
private:
	uint spaceId;
	uint moduleId;
	bool disposed;
	char* commandLine;
	char* environment;
	PeModule* module;
	int exitCode;
	ProcessInformation* pi;
	int threadCount;
	char modulePath[PATH_LEN];
	Process *prev, *next, *parent, *child;
	static Process* EnumProcessTree( Process* p, uint id );
	static void KillProcessChildren( Process* p );
public:
	Thread* mainThread;
	static Process* GetProcessById( uint id );
	Thread* GetThreadById( uint tid );
	Process( uint pid, char* cmdline, char* env );
	~Process();
	int Initialize( uint pid, char* cmdline, char* env );
	void Terminate( int code );
	void Resume();
	void Suspend();
	Thread* CreateThread( size_t entry );
	void Dispose();
	uint SpaceId(){
		return spaceId;
	}
	uint ProcessId(){
		return spaceId;
	}
	PeModule* GetModule(){
		return module;
	}
	ProcessInformation* GetInformation(){
		return pi;
	}
};

#endif
