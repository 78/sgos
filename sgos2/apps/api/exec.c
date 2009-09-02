#include <sgos.h>
#include <api.h>

int __do_execute( PROCESS_INFO* pi )
{
	int ret;
	uint module;
	char* cmdline = pi->cmdline;
	//parse cmdline & variables
	//then try some path to load the module.
	ret = sys_loader_open( cmdline, &module );
	if( ret < 0 )
		return ret;
	return 0;
}
