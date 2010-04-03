#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>

ServiceInformation * si, *readOnlyAddress;

ServiceInformation* LookupById( uint id )
{
	if( id< SI_MAX && si[id].ServiceId )
			return &si[id];
	return 0;
}

ServiceInformation* LookupByName( const char* name )
{
	int i;
	for( i=0; i<SI_MAX; i++ )
		if( strncmp(si[i].ServiceName, name, SERVICE_NAME_LENGTH) == 0 )
			return &si[i];
	return 0;
}

ServiceInformation* AddService( uint id, uint event, uint tid, const char* name )
{
	if( id >= SI_MAX )	
		return 0;
	if( id==0 || si[id].ServiceId ){
		for( id=0; id<SI_MAX; id++ )
			if( si[id].ServiceId == 0 )
				break;
		if( id == SI_MAX )
			return 0;
	}
	si[id].EventFlag = event;
	si[id].ThreadId = tid;
	memcpy( si[id].ServiceName, name, SERVICE_NAME_LENGTH);
	si[id].ServiceId = id;
	printf("[servicemanager]AddService %d:%x:%s\n", id, tid,
		name );
	return &si[id];
}

static void DelService( uint id )
{
	if( id< SI_MAX )
		si[id].ServiceId = 0;
}

void ServiceMessageLoop()
{
	Message msg = {0};
	int result;
	printf("[servicemanager]Program started.\n");
	for( ;; ){
		memset( &msg, 0, sizeof(msg) );
		result = WaitMessage( &msg );
		if( result < 0 ){
			printf("[servicemanager] Failed in Api_Receive: result = %d\n", result );
			continue;
		}
		msg.Code = 0; //set it to success
		switch( msg.Command ){
			case Service_Notify:
			{
				ServiceInformation* s = AddService(
					msg.Arguments[0],	//ServiceId
					msg.Arguments[1],	//event
					msg.Arguments[2], 	//ThreadId
					(const char*)&msg.Arguments[3]	//ServiceName
					);
				if( s == NULL ){
					msg.Code = -ERR_NOMEM;
				}else{
					msg.Code = s->ServiceId;
				}
				break;
			}
			case Service_Remove:
				DelService( msg.Arguments[0] ); //remove by ServiceId
				break;
			case Service_LookupById:
			{
				ServiceInformation* s = LookupById( 
					msg.Arguments[0] 	//ServiceId
					);
				if( s == NULL ){
					msg.Code = -ERR_NONE;
				}else{
					msg.Arguments[1] = s->EventFlag; 	//event
					msg.Arguments[2] = s->ThreadId; 	//ThreadId
					memcpy( &msg.Arguments[3], s->ServiceName, SERVICE_NAME_LENGTH ); //ServiceName
				}
				break;
			}
			case Service_LookupByName:
			{
				ServiceInformation* s = LookupByName( 
					(const char*)&msg.Arguments[3] 	//ServiceName
					);
				if( s == NULL ){
					msg.Code = -ERR_NONE;
				}else{
					msg.Arguments[0] = s->ServiceId; 	//ServiceId
					msg.Arguments[1] = s->EventFlag; 	//event
					msg.Arguments[2] = s->ThreadId; 	//ThreadId
				}
				break;
			}
		}
		ReplyMessage( &msg );
	}
}


/*
	Allocate one or more global pages to store the service informations.
*/
int main()
{
	int result;
	uint phys_addr, attr;
	uint sid = SysGetCurrentSpaceId();
	printf("[servicemanager]SpaceId:%X\n", sid );
	si = (ServiceInformation*)SysAllocateMemory( SysGetCurrentSpaceId(), SM_INFORMATION_SIZE, MEMORY_ATTR_WRITE, ALLOC_VIRTUAL );
	//Allocate two read-only pages. 
	readOnlyAddress = ((SystemInformation*)SysGetSystemInformation())->ServiceList;
	//Get physical page of readOnlyAddress
	result = SysQueryMemory( sid, (size_t)readOnlyAddress, (size_t*)&phys_addr, &attr );
	if( result <0 ){
		printf("[servicemanager]Failed to get physical page of readOnlyAddress: result=%d\n", result);
		SysExitSpace(result);
	}
	result = SysMapMemory( sid, (size_t)si, SM_INFORMATION_SIZE, phys_addr, 0, MAP_ADDRESS );
	if( result <0 ){
		printf("[servicemanager]Failed to map the physical page\n");
		SysExitSpace(result);
	}
	//Add Me
	AddService( 1, 0, SysGetCurrentThreadId(), "ServiceManager" );
	result = readOnlyAddress[1].ServiceId;
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}
