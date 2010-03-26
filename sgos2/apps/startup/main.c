#include <api.h>
#include <stdio.h>

void run( const char* p )
{
	uint pid;
	if( !*p )
		return;
	printf("[startup] running %s\n", p );
	int ret = PsCreateProcess( p, "", &pid );
	if( ret < 0 )
		printf("[startup] Failed to run %s\n", p );
}

void parse( char* buf )
{
	char* p, *q;
	for( p=q=buf; ; q++){
		if( *q=='\0' || *q=='#' ){
			*q = '\0';
			run( p );
			break;
		}
		if( *q=='\r' || *q=='\n' ){
			*q='\0';
			run( p );
			p = q+1;
		}
	}
}

int main()
{
	FILEBUF* fb=0; 
	char buf[4096];
	int ret; 
	char *path = "/c:/sgos2/startup.txt";
	fb = FsOpenFile( path, FILE_READ, 0);
	if( fb == NULL ){
		printf("[startup] Cannot open file %s\n", path );
	}
	ret = FsReadFile(fb, (uchar*)buf, 4096 );
	FsCloseFile( fb ); 
	if( ret < 0 )
		return ret;
	buf[ret]=0;
	parse( buf );
	return 0;
}

