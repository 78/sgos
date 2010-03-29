#include <sgos.h>
#include <stdio.h>
#include <api.h>


int execve(const char * filename,char * const argv[ ],char * const envp[ ])
{
	int pid, i;
	char* e=NULL, *t =NULL;
	t = (char*)malloc( PAGE_SIZE );
	//FIXME: overflow...
	strcpy( t, filename );
	for( i=0; argv && argv[i]; i++ ){
		strcat( t, " " );
		strcat( t, argv[i] );
	}
	e = (char*)malloc( ENVIRONMENT_STRING_SIZE );
	if( !e )
		goto bed;
	e[0] = '\0';
	for( i=0; envp && envp[i]; i++ ){
		strcat( e, envp[i] );
		strcat( t, ";" );
	}
	if( PsCreateProcess( t, e, &pid ) < 0 )
		goto bed;
	return pid;
bed:
	if( e ) free( e );
	if( t ) free( t );
	return -ERR_UNKNOWN;
}

int execvp(const char *file ,char * const argv [])
{
	return execve( file, argv, NULL );
}
