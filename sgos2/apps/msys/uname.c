#include <sgos.h>
#include "debug.h"
#include "cygwin.h"

struct utsname
{
	char sysname[20];
	char nodename[20];
	char release[20];
	char version[20];
	char machine[20];
};

int uname( struct utsname* name )
{
	memset( name, 0, sizeof(*name) );
	sprintf( name->sysname, "MSYS32_SGOS-%d.%d", OS_MAJOR_VERSION, OS_MINOR_VERSION );
	strcpy( name->nodename, "unknown" );
	sprintf( name->release, "GCC%.16s", __VERSION__ );
	sprintf( name->version, "%s", __DATE__ );
	strcpy( name->machine, "unknown" );
	return 0;
}

