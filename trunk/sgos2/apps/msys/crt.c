#include <sgos.h>
#include <api.h>
#include <stdlib.h>
#include "cygwin.h"
#include "debug.h"
#include "reent.h"

/* Pointer into application's static data */
struct per_process __cygwin_user_data;
#define user_data (&__cygwin_user_data)

char*** main_environ;

struct _reent reent_data = _REENT_INIT(reent_data);

void __main (void)
{
}

uint cygwin_internal( int type )
{
	switch( type ){
	case CW_USER_DATA:
		memset( user_data, 0, sizeof( __cygwin_user_data ) );
		if( user_data->resourcelocks==NULL )
			user_data->resourcelocks = (void*)0x26666666;
		if( user_data->threadinterface==NULL )
			user_data->threadinterface = (void*)0x36666666;
		if( user_data->impure_ptr==NULL )
			user_data->impure_ptr = (void*)&reent_data;
		DBG("return : %X" , user_data );
		return (uint)user_data;
	}
}


void dll_crt0__FP11per_process(struct per_process* u)
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	int argc, envc;
	char** argv, **envv;
	char* cmd;
	u = user_data;
	main_environ = user_data->envptr;
	//init environ
	build_argv( pi->EnvironmentVariables, &envv, &envc, ':' );
	*main_environ = envv;
	//init command arguments
	cmd = pi->CommandLine;
	DBG("cmdline: %s", cmd );
	build_argv( cmd, &argv, &argc, ' ' );
	u->main( argc, argv, *main_environ );
}

