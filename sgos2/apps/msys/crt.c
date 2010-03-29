#include <sgos.h>
#include <api.h>
#include <stdlib.h>
#include "cygwin.h"
#include "debug.h"


typedef int ResourceLocks ;
typedef int MTinterface ;
ResourceLocks _reslock ;
MTinterface _mtinterf;
void* reent_data;
/* Pointer into application's static data */
struct per_process __cygwin_user_data;
#define user_data (&__cygwin_user_data)

char*** main_environ;

void __main (void)
{
}

uint cygwin_internal( int type )
{
	switch( type ){
	case CW_USER_DATA:
		memset( user_data, 0, sizeof( __cygwin_user_data ) );
		DBG("return : %X" , user_data );
		return (uint)user_data;
	}
}

static void build_cmdargv (char *cmd, char ***argv_ptr, int *argc)
{
	if( !cmd ){
		DBG("cmd==NULL");
		return;
	}
	char* p, *q;
	*argc = 1;
	int i=0;
	for(p=cmd; *p; p++ )
		if( *p == ' ' )
			*argc++;
	*argv_ptr = (char**)malloc( sizeof(char*) * (*argc+1) );
	if( !*argv_ptr ){
		DBG("cannot allocate memory.");
		return;
	}
	for(p=q=cmd; ; p++ ){
		if( *p==' ' || !*p ){
			int len = (int)p-(int)q;
			(*argv_ptr)[i] = (char*)malloc( len + 1 );
			memcpy( (*argv_ptr)[i], q, len );
			(*argv_ptr)[i][len] = '\0';
			if( !*p )
				break;
			q = p;
		}
	}
}

void dll_crt0__FP11per_process(struct per_process* u)
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	int argc;
	char** argv;
	char* cmd;
	u = user_data;
	main_environ = user_data->envptr;
	//init environ
	cmd = pi->CommandLine;
	build_cmdargv( cmd, &argv, &argc );
	*main_environ = argv;
	DBG("argc: %d", argc );
	u->main( argc, argv, *main_environ );
}

