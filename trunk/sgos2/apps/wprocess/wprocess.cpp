#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>


class Process{
	uint spaceId = 0;
	uint moduleId;
	uint mainThreadId;
	bool disposed = false;
	char* commandLine = NULL;
	char* environment = NULL;
	
	Process( char* cmdline, char* env ){
		Create( cmdline, env );
	}
	
	~Process(){
		if( this->disposed )
			return;
		this->disposed = true;
		if( this->spaceId ){
			this->Terminate(-1);
			this->spaceId = 0;
		}
		//release resources
		if( this->commandLine )
			FreeMemory( GetCurrentSpaceId(), this->commandLine );
		if( this->environment )
			FreeMemory( GetCurrentSpaceId(), this->environment );
		this->commandLine = NULL;
		this->environment = NULL;
	}
	
	// How to create a process ?
	int Create( char* cmdline, char* env ){
		int tid;
		size_t entry;
		//1. create a space for the process
		int sid = CreateSpace( NULL );
		if( sid < 0 )
			return sid;
		//2. setup evironment
		// we don't need to copy env strings to the new process, it will ask
		// us for them later.
		// ... etc
		//3. load executable file to the space!
		LoadLibrary( sid, cmdline );
		//4. setup process information block  Pib
		//5. create a main thread
		tid = CreateThread( sid, entry );
		//6. setup thread information block Tib
		
		//7. save informations
		this->mainThreadId = tid;
		this->spaceId = sid;
		this->commandLine = cmdline;
		this->environment = env;
		//8. start main thread to start the process
		ResumeThread( tid );
	}
	
	int Terminate( int code ){
		
	}
	
	int Suspend(){
	}
	
	int Resume(){
	}
};
