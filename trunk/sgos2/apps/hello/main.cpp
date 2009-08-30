#include <system.h>
#include <stdio.h>

using namespace System;

int main()
{
	Messenger sender;
	THREAD_CONTEXT tc;
	int oldMode;
	printf("[Hello]Now tests the BIOS Call & speaker service!\n");
	Thread::Sleep(2000);
	//映射前1MB
	Service::MapMemory( 0, 0, 1<<20, 0 );
	tc.ds = tc.es = 0x1000;
	//保存当前模式
	tc.eax = 0x4F03;
	Service::CallBIOS( 0x10, &tc );
	if( tc.eax != 0x4F )
		printf("[Hello]Failed to get current resolution.\n");
	Thread::Sleep(1000);
	oldMode = tc.ebx;
	//设置分辨率为640*480*16
	tc.eax = 0x4F02;
	tc.ebx = 0x111;
	Service::CallBIOS( 0x10, &tc );
	if( tc.eax != 0x4F )
		printf("[Hello]Failed to change resolution.\n");
	Thread::Sleep(2000);
	tc.eax = 0x4F02;
	tc.ebx = oldMode;
	Service::CallBIOS( 0x10, &tc );
	
	/* Test 1 */
	printf("[Hello]First, send a beep message.\n");
	Thread::Sleep(1000);
	sender.parse("<msg to=\"speaker\"><beep /></msg>");
	sender.send();
	Thread::Sleep(2000);
	/* Test 2 */
	printf("[Hello]Then, send a play message, frequency=1000, delay=500ms\n");
	Thread::Sleep(1000);
	sender.parse("<msg to=\"speaker\"><play /></msg>");
	sender.putUInt( "/play:freq", 1000 );
	sender.putUInt( "/play:delay", 500 );
	sender.send();
	Thread::Sleep(2000);
	/* Test 3 */
	printf("[Hello]At last, send a test message.\n");
	Thread::Sleep(1000);
	sender.parse("<msg to=\"speaker\"><test /></msg>");
	sender.send();
	Thread::Sleep(5000);
	
	printf("[Hello]Test over.\n");
	return 0;
}

