#include <system.h>
#include <stdio.h>

using namespace System;

int main()
{
	Messenger sender;
	printf("[Hello]Now tests the speaker service!\n");
	Thread::Sleep(2000);
	
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

