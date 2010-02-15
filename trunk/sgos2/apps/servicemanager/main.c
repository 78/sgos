#include <sgos.h>
#include <api.h>
#include <string.h>

#define SI_MAX   (SM_INFORMATION_SIZE/sizeof(ServiceInformation))
ServiceInformation * si, *readOnlyAddress;

ServiceInformation* LookupById( uint id )
{
	if( id< SI_MAX && si[id].ServiceId )
			return &si[id];
	return NULL;
}

ServiceInformation* LookupByName( const char* name )
{
	int i;
	for( i=0; i<SI_MAX; i++ )
		if( strncmp(si[i].ServiceName, name, SERVICE_NAME_LENGTH) == 0 )
			return &si[i];
	return NULL;
}

ServiceInformation* AddService( uint id, uint event, uint tid, const char* name )
{
	if( id >= SI_MAX )	
		return NULL;
	if( si[id].ServiceId ){
		for( id=0; id<SI_MAX; id++ )
			if( si[id].ServiceId == 0 )
				break;
		if( id == SI_MAX )
			return NULL;
	}
	si[id].EventFlag = event;
	si[id].ThreadId = tid;
	memcpy( si[id].ServiceName, name, SERVICE_NAME_LENGTH);
	si[id].ServiceId = id;
}

void RemoveService( uint id )
{
	if( id< SI_MAX )
		si[id].ServiceId = 0;
}

void ServiceMessageLoop()
{
	Message msg = {0};
	int result;
	printf("[servicemanager] program started.\n");
	for( ;; ){
		result = ReceiveMessage( &msg, INFINITE );
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
				if( s == NULL )
					msg.Code = -ERR_NOMEM;
				break;
			}
			case Service_Remove:
				RemoveService( msg.Arguments[0] ); //remove by ServiceId
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
int ServiceEntry()
{
	int result;
	uint phys_addr, attr;
	uint sid = GetCurrentSpaceId();
	printf("[servicemanager]SpaceId:%X\n", sid );
	//Allocate two read-only pages. 
	readOnlyAddress = (void*)AllocateGlobalMemory( SM_INFORMATION_SIZE, 0, ALLOC_VIRTUAL );
	if( readOnlyAddress==NULL ){
		printf("[servicemanager]Failed to allocate global memory\n");
		ExitSpace(result);
	}
	printf("[servicemanager]Got global memory. Now map it to local memory\n");
	//Allocate virtual memory in current address space
	si = (void*)AllocateMemory( sid, SM_INFORMATION_SIZE, MEMORY_ATTR_WRITE, 0 );
	if( si == NULL ){
		printf("[servicemanager]Failed to allocate virtual memory\n");
		ExitSpace(result);
	}
	//Get physical page of readOnlyAddress
	result = QueryMemory( sid, (size_t)si, &phys_addr, &attr );
	if( result <0 ){
		printf("[servicemanager]Failed to get physical page of readOnlyAddress: result=%d\n", result);
		ExitSpace(result);
	}
	result = MapMemory( sid, (size_t)readOnlyAddress, phys_addr, SM_INFORMATION_SIZE, 0, MAP_ADDRESS );
	if( result <0 ){
		printf("[servicemanager]Failed to map the physical page of readOnlyAddress\n");
		ExitSpace(result);
	}
	//Initialize si to zeros
	memset( si, 0, SM_INFORMATION_SIZE );
	//Add Me
	AddService( 0, 0, GetCurrentThreadId(), "ServiceManager" );
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}
