#include <sgos.h>
//overload the operator "new"
extern "C" unsigned * get_ctors();
extern "C" unsigned * get_dtors();

//for MINGW
extern "C" void __main()
{
}
extern "C" int main();
extern "C" void __run_exit_func();
extern "C" void __allocation_init();
extern "C" void SysExitThread(uint );
extern "C" ProcessInformation* GetCurrentProcessInformation();
extern "C" int mainCRTStartup()
{
	int ret;
	unsigned *p;
	// init memory
	__allocation_init();
	// call ctors
	p = get_ctors();
	for( p=&p[1]; *p; p++ )
		((void(*)())(*p))();
	ret = main();
	// call atexit
	__run_exit_func();
	// call dtors
	p = get_dtors();
	for( p=&p[1]; *p; p++ )
		((void(*)())(*p))();
	SysExitThread((uint)ret);
	// return
	return ret;
}
