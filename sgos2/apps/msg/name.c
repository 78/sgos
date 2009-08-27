//Namespace

#include <sgos.h>
#include <api.h>

int create_name( uint thread, const char* name )
{
	return sys_namespace_register( thread, (char*)name );
}

int delete_name( uint thread, const char* name )
{
	return sys_namespace_unregister( thread, (char*)name );
}

uint match_name( const char* name )
{
	return sys_namespace_match( (char*)name );
}

