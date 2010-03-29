#ifndef _HANDLE_

#include <sgos.h>

enum HANDLE_TYPE{
	HANDLE_UNKNOWN = 0,
	HANDLE_DISK = 0x0001,
	HANDLE_CHAR = 0x0002,
	HANDLE_PIPE = 0x0003,
	HANDLE_MODULE = 0x0004,
	HANDLE_FILE,
	HANDLE_PROCESS,
	HANDLE_THREAD,
	HANDLE_EVENT
};

uint create_handle( int type, uint data );
void close_handle( uint h );
uint get_handle_data( uint h );
uint get_handle_type(uint h );

#endif
