//Namespace

#include <sgos.h>
#include <api.h>

int create_name( uint thread, const char* name )
{
	return sys_namespace_create( thread, (char*)name );
}

int delete_name( uint thread, const char* name )
{
	return sys_namespace_delete( thread, (char*)name );
}

uint match_name( const char* name )
{
	return sys_namespace_match( (char*)name );
}

