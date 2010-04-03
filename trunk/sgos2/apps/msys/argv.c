#include <sgos.h>
#include "debug.h"

void build_argv (char *cmd, char ***argv_ptr, int *argc, char sep)
{
	if( !cmd ){
		DBG("cmd==NULL");
		return;
	}
	char* p, *q;
	*argc = 1;
	int i=0;
	for(p=cmd; *p; p++ )
		if( *p == sep )
			(*argc)++;
	*argv_ptr = (char**)malloc( sizeof(char*) * (*argc+1) );
	if( !*argv_ptr ){
		DBG("cannot allocate memory. size=0x%x", sizeof(char*) * (*argc+1) );
		return;
	}
	for(p=q=cmd; ; p++ ){
		if( *p==sep || !*p ){
			int len = (int)p-(int)q;
			(*argv_ptr)[i] = (char*)malloc( len + 1 );
			if( (*argv_ptr)[i] == NULL )
				return;
			memcpy( (*argv_ptr)[i], q, len );
			(*argv_ptr)[i][len] = '\0';
			i++;
			if( !*p )
				break;
			q = p+1;
		}
	}
}
